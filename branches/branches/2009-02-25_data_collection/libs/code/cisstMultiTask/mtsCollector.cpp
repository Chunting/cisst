/*
  $Id: mtsCollector.cpp 2009-03-02 mjung5

  Author(s):  Min Yang Jung
  Created on: 2009-02-25

  (C) Copyright 2008-2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstMultiTask/mtsCollector.h>

CMN_IMPLEMENT_SERVICES(mtsCollector)

unsigned int mtsCollector::CollectorCount;
mtsTaskManager * mtsCollector::taskManager;

//-------------------------------------------------------
//	Constructor, Destructor, and Initializer
//-------------------------------------------------------
mtsCollector::mtsCollector(const std::string & collectorName, double period) : 	
    TimeOffsetToZero(false),
    mtsTaskPeriodic(collectorName, period, false)
{
    ++CollectorCount;

    if (taskManager == NULL) {
        taskManager = mtsTaskManager::GetInstance();
    }

    Init();

    // By default, a data collection period is set as the period of this task.
    CollectingPeriod = period;
}

mtsCollector::~mtsCollector()
{
    --CollectorCount;

    CMN_LOG_CLASS(5) << "Collector " << GetName() << " ends." << std::endl;
}

void mtsCollector::Init()
{
    TimeOffsetToZero = false;

    Status = COLLECTOR_STOP;
    DelayedStart = 0.0;
    DelayedStop = 0.0;
    CollectingPeriod = 0.0;
    CollectingStride = 0;

    ClearTaskMap();    
}

//-------------------------------------------------------
//	Thread management functions (called internally)
//-------------------------------------------------------
void mtsCollector::Startup(void)
{
    // initialization
    Init();

    // By default, a data collection period is set as the period of this task.
    CollectingPeriod = mtsTaskPeriodic::Period;
}

void mtsCollector::Run(void)
{
    if (!IsAnySignalRegistered()) return;

    Collect();
}

void mtsCollector::Cleanup(void)
{
    // clean up
    ClearTaskMap();
}

//-------------------------------------------------------
//	Signal Management
//-------------------------------------------------------
bool mtsCollector::AddSignal(const std::string & taskName, 
                             const std::string & signalName, 
                             const std::string & format)
{	
    CMN_ASSERT(taskManager);

    // Ensure that the specified task is under the control of the task manager.
    mtsTask * task = taskManager->GetTask(taskName);
    if (task == NULL) {
        CMN_LOG_CLASS(5) << "Unknown task: " << taskName << std::endl;
        return false;
    }

    // Prevent duplicate signal registration
    if (FindSignal(taskName, signalName)) {
        CMN_LOG_CLASS(5) << "Already registered signal: " << taskName 
            << ", " << signalName << std::endl;

        //return false;
        throw mtsCollector::mtsCollectorException(
            "Already collecting signal: " + taskName + ", " + signalName);
    }

    // Double check: check if the specified signal has been already added to the state table.
    if (task->GetStateVectorID(signalName) >= 0) {  // 0: Toc, 1: Tic, 2: Period, >3: user
        CMN_LOG_CLASS(5) << "Already registered signal: " << taskName
            << ", " << signalName << std::endl;

        //return false;
        throw mtsCollector::mtsCollectorException(
            "Already collecting signal: " + taskName + ", " + signalName);
    }

    // Add a signal
    SignalMapElement element;
    element.Task = task;

    TaskMap::iterator itr = taskMap.find(taskName);
    if (itr == taskMap.end()) {    // If the task is new
        // Create a new instance of SignalMap
        SignalMap * signalMap = new SignalMap();
        CMN_ASSERT(signalMap);
        
        signalMap->insert(make_pair(signalName, element));
        taskMap.insert(make_pair(taskName, signalMap));
    } else {    // If the task has one or more signals being collected
        itr->second->insert(make_pair(signalName, element));
    }

    CMN_LOG_CLASS(5) << "Signal added: " << taskName << ", " << signalName << std::endl;

    return true;
}

bool mtsCollector::RemoveSignal(const std::string & taskName, const std::string & signalName)
{
    // If no signal data is being collected.
    if (taskMap.empty()) {
        CMN_LOG_CLASS(5) << "No signal data is being collected." << std::endl;
        return false;
    }

    // If finding a nonregistered task or signal
    if (!FindSignal(taskName, signalName)) {
        CMN_LOG_CLASS(5) << "Unknown task and/or signal: " << taskName 
            << ", " << signalName << std::endl;
        return false;
    }

    // Remove a signal from the list
    TaskMap::iterator itr = taskMap.find(taskName);
    CMN_ASSERT(itr != taskMap.end());

    SignalMap * signalMap = itr->second;
    SignalMap::iterator _itr = signalMap->find(signalName);
    CMN_ASSERT(_itr != signalMap->end());
    signalMap->erase(_itr);

    // Clean-up
    if (signalMap->empty()) {
        delete signalMap;
        taskMap.erase(itr);
    }

    CMN_LOG_CLASS(5) << "Signal removed: " << taskName << ", " << signalName << std::endl;

    return true;
}

bool mtsCollector::FindSignal(const std::string & taskName, const std::string & signalName)
{
    if (taskMap.empty()) return false;

    TaskMap::const_iterator itr = taskMap.find(taskName);
    if (itr == taskMap.end()) {
        return false;
    } else {
        SignalMap * signalMap = itr->second;
        SignalMap::const_iterator _itr = signalMap->find(signalName);
        if (_itr == signalMap->end()) {
            return false;
        } else {
            return true;
        }
    }
}

//-------------------------------------------------------
//	Collecting Data
//-------------------------------------------------------
void mtsCollector::SetTimeBase(const double deltaT, const bool offsetToZero)
{
    // Ensure that there is a task being collected.
    if (taskMap.empty()) {
        return;
    }

    // deltaT should be positive.
    if (deltaT <= 0.0) {
        return;
    }

    CollectingPeriod = deltaT;
    CollectingStride = 0;
    TimeOffsetToZero = offsetToZero;
}

void mtsCollector::SetTimeBase(const unsigned int deltaStride, const bool offsetToZero)
{
    // Ensure that there is a task being collected.
    if (taskMap.empty()) {
        return;
    }

    // deltaStride should be positive.
    if (deltaStride <= 0) {
        return;
    }

    CollectingPeriod = 0.0;
    CollectingStride = deltaStride;
    TimeOffsetToZero = offsetToZero;
}

void mtsCollector::Start(const double delayedStart)
{    
    // Check for state transition
    switch (Status) {
        case COLLECTOR_WAIT_START:
            CMN_LOG_CLASS(5) << "Waiting for the collector to start." << std::endl;
            return;

        case COLLECTOR_WAIT_STOP:
            CMN_LOG_CLASS(5) << "Waiting for the collector to stop." << std::endl;
            return;

        case COLLECTOR_COLLECTING:
            CMN_LOG_CLASS(5) << "The collector is now running." << std::endl;
            return;
    }

    DelayedStart = delayedStart;
    Status = COLLECTOR_WAIT_START;

    Stopwatch.Reset();
    Stopwatch.Start();
}

void mtsCollector::Stop(const double delayedStop)
{
    // Check for state transition
    switch (Status) {
        case COLLECTOR_WAIT_START:
            CMN_LOG_CLASS(5) << "Waiting for the collector to start." << std::endl;
            return;

        case COLLECTOR_WAIT_STOP:
            CMN_LOG_CLASS(5) << "Waiting for the collector to stop." << std::endl;
            return;

        case COLLECTOR_STOP:
            CMN_LOG_CLASS(5) << "The collector is not running." << std::endl;
            return;
    }

    DelayedStop = delayedStop;
    Status = COLLECTOR_WAIT_STOP;

    Stopwatch.Reset();
    Stopwatch.Start();
}

void mtsCollector::Collect()
{
    if (Status == COLLECTOR_STOP) return;

    // Check for the state transition
    switch (Status) {
        case COLLECTOR_WAIT_START:
            if (Stopwatch.IsRunning()) {
                if (Stopwatch.GetElapsedTime() < DelayedStart) {
                    return;
                } else {
                    // Start collecting
                    DelayedStart = 0.0;
                    Status = COLLECTOR_COLLECTING;
                    Stopwatch.Stop();
                }
            } else {
                return;
            }
            break;

        case COLLECTOR_WAIT_STOP:
            if (Stopwatch.IsRunning()) {
                if (Stopwatch.GetElapsedTime() < DelayedStop) {
                    return;
                } else {
                    // Stop collecting
                    DelayedStop = 0.0;
                    Status = COLLECTOR_STOP;
                    Stopwatch.Stop();
                }
            } else {
                return;
            }
            break;
    }

    CMN_ASSERT(Status == COLLECTOR_COLLECTING ||
               Status == COLLECTOR_STOP);

    if (Status == COLLECTOR_STOP) {
        DelayedStop = 0.0;
        CMN_LOG_CLASS(3) << "The collector stopped." << std::endl;
        return;
    }

    // Begins collecting data from registered tasks and signals
    TaskMap::iterator itr = taskMap.begin();
    SignalMap::iterator _itr;
    SignalMap * signalMap = NULL;
    mtsCollectorBuffer * Buffer = NULL;
    mtsTask * Task = NULL;

    CMN_ASSERT(itr != taskMap.end());

    // Iterate registered signals sequentially
    for (; itr != taskMap.end(); ++itr) {
        signalMap = itr->second;
        _itr = signalMap->begin();
        for (; _itr != signalMap->end(); ++_itr) {
            Task = _itr->second.Task;
            Buffer = &_itr->second.Buffer;

            CMN_ASSERT(Task->GetStateVectorID(_itr->first));

            // Sample data
            // Store
        }
    }
    
    // 4. file output (or display visually)

    /*
    // the state table provides an index
    const mtsStateIndex now = StateTable.GetIndexWriter();

    // process the commands received, i.e. possible SetSineAmplitude
    ProcessQueuedCommands();

    // compute the new values based on the current time and amplitude
    SineData = SineAmplitude.Data
        * sin(2 * cmnPI * static_cast<double>(now.Ticks()) * Period / 10.0);
    */
}

//-------------------------------------------------------
//	Miscellaneous Functions
//-------------------------------------------------------
void mtsCollector::ClearTaskMap(void)
{
    if (!taskMap.empty()) {        
        TaskMap::iterator itr = taskMap.begin();
        SignalMap::iterator _itr;
        for (; itr != taskMap.end(); ++itr) {
            itr->second->clear();
            delete itr->second;
        }

        taskMap.clear();
    }
}