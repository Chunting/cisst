/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/* $Id$ */

#ifndef _appTask_h
#define _appTask_h

#include <cisstMultiTask.h>
#include "appTaskUI.h"
#include "robotLowLevel.h"

//***** Example of using cisstMultiTask without command pattern. *****

class appTask: public mtsTaskPeriodic {

    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR);
    volatile bool ExitFlag;
    
 protected:
    unsigned long Ticks;

    robotLowLevel* controlledTask;
    robotLowLevel* observedTask;

    AppTaskUI ui;

 public:
    appTask(const std::string & taskName,
            const std::string & controlledRobot,
            const std::string & observedRobot,
            double period);
    ~appTask() {};
    void Configure(const std::string & CMN_UNUSED(filename) = "") {};
    void Startup(void);
    void Run(void);
    void Cleanup(void) {};

    inline bool GetExitFlag (void) const { return ExitFlag;}

    // callback to close
    static void Close(mtsTask * task);
};

CMN_DECLARE_SERVICES_INSTANTIATION(appTask);

#endif // _appTask_h

/*
  Author(s):  Peter Kazanzides, Anton Deguet
  Created on: 2004-04-30

  (C) Copyright 2004-2008 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/