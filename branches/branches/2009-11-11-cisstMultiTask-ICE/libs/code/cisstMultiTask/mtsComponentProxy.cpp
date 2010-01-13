/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsComponentProxy.cpp 291 2009-04-28 01:49:13Z mjung5 $

  Author(s):  Min Yang Jung
  Created on: 2009-12-18

  (C) Copyright 2009-2010 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstMultiTask/mtsComponentProxy.h>

#include <cisstCommon/cmnUnits.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <cisstMultiTask/mtsRequiredInterface.h>

#include <cisstMultiTask/mtsCommandVoidProxy.h>
#include <cisstMultiTask/mtsCommandWriteProxy.h>
#include <cisstMultiTask/mtsCommandReadProxy.h>
#include <cisstMultiTask/mtsCommandQualifiedReadProxy.h>
#include <cisstMultiTask/mtsMulticastCommandVoid.h>
#include <cisstMultiTask/mtsMulticastCommandWriteProxy.h>

#include <cisstMultiTask/mtsFunctionVoid.h>
#include <cisstMultiTask/mtsFunctionReadOrWrite.h>
#include <cisstMultiTask/mtsFunctionQualifiedReadOrWrite.h>

#include <sstream>

CMN_IMPLEMENT_SERVICES(mtsComponentProxy);

const std::string ComponentInterfaceCommunicatorID = "ComponentInterfaceServerSender";

mtsComponentProxy::~mtsComponentProxy()
{
    // Stop all the provided interface proxies running.
    ProvidedInterfaceProxyMapType::MapType::iterator itProvidedProxy = ProvidedInterfaceProxies.GetMap().begin();
    const ProvidedInterfaceProxyMapType::MapType::const_iterator itProvidedProxyEnd = ProvidedInterfaceProxies.end();
    for (; itProvidedProxy != itProvidedProxyEnd; ++itProvidedProxy) {
        itProvidedProxy->second->Stop();
    }

    // Stop all required interface proxies running.
    //RequiredInterfaceProxyMapType::MapType::iterator it2 =
    //    RequiredInterfaceProxies.GetMap().begin();
    //for (; it2 != RequiredInterfaceProxies.GetMap().end(); ++it2) {
    //    it2->second->Stop();
    //}

    osaSleep(500 * cmn_ms);

    // Deallocation
    ProvidedInterfaceProxies.DeleteAll();
    //RequiredInterfaceProxies.DeleteAll();

    //FunctionVoidProxyMap.DeleteAll();
    //FunctionWriteProxyMap.DeleteAll();
    //FunctionReadProxyMap.DeleteAll();
    //FunctionQualifiedReadProxyMap.DeleteAll();
    //EventVoidGeneratorProxyMap.DeleteAll();
    //EventWriteGeneratorProxyMap.DeleteAll();
}

//-----------------------------------------------------------------------------
//  Methods for Server Components
//-----------------------------------------------------------------------------
bool mtsComponentProxy::CreateRequiredInterfaceProxy(RequiredInterfaceDescription & requiredInterfaceDescription)
{    
    const std::string requiredInterfaceName = requiredInterfaceDescription.RequiredInterfaceName;

    // Check if the interface name is unique
    mtsRequiredInterface * requiredInterface = GetRequiredInterface(requiredInterfaceName);
    if (requiredInterface) {
        CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: can't create required interface proxy: "
            << "duplicate name: " << requiredInterfaceName << std::endl;
        return false;
    }

    // Create a required interface proxy
    bool success;
    mtsRequiredInterface * requiredInterfaceProxy = new mtsRequiredInterface(requiredInterfaceName);

    // Populate the new required interface
    mtsFunctionVoid * functionVoidProxy;
    mtsFunctionWrite * functionWriteProxy;
    mtsFunctionRead * functionReadProxy;
    mtsFunctionQualifiedRead * functionQualifiedReadProxy;

    // Create void function proxies
    const std::vector<std::string> namesOfFunctionVoid = requiredInterfaceDescription.FunctionVoidNames;
    for (unsigned int i = 0; i < namesOfFunctionVoid.size(); ++i) {
        //functionVoidProxy = new mtsFunctionVoid(providedInterface, namesOfFunctionVoid[i]);
        functionVoidProxy = new mtsFunctionVoid();
        success = requiredInterfaceProxy->AddFunction(namesOfFunctionVoid[i], *functionVoidProxy);
        //success &= FunctionVoidProxyMap.AddItem(namesOfFunctionVoid[i], functionVoidProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add void function proxy: " << namesOfFunctionVoid[i] << std::endl;
            return false;
        }
    }

    // Create write function proxies
    const std::vector<std::string> namesOfFunctionWrite = requiredInterfaceDescription.FunctionWriteNames;
    for (unsigned int i = 0; i < namesOfFunctionWrite.size(); ++i) {
        //functionWriteProxy = new mtsFunctionWrite(providedInterface, namesOfFunctionWrite[i]);
        functionWriteProxy = new mtsFunctionWrite();
        success = requiredInterfaceProxy->AddFunction(namesOfFunctionWrite[i], *functionWriteProxy);
        //success &= FunctionWriteProxyMap.AddItem(namesOfFunctionWrite[i], functionWriteProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add write function proxy: " << namesOfFunctionWrite[i] << std::endl;
            return false;
        }
    }

    // Create read function proxies
    const std::vector<std::string> namesOfFunctionRead = requiredInterfaceDescription.FunctionReadNames;
    for (unsigned int i = 0; i < namesOfFunctionRead.size(); ++i) {
        //functionReadProxy = new mtsFunctionRead(providedInterface, namesOfFunctionRead[i]);
        functionReadProxy = new mtsFunctionRead();
        success = requiredInterfaceProxy->AddFunction(namesOfFunctionRead[i], *functionReadProxy);
        //success &= FunctionReadProxyMap.AddItem(namesOfFunctionRead[i], functionReadProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add read function proxy: " << namesOfFunctionRead[i] << std::endl;
            return false;
        }
    }

    // Create QualifiedRead function proxies
    const std::vector<std::string> namesOfFunctionQualifiedRead = requiredInterfaceDescription.FunctionQualifiedReadNames;
    for (unsigned int i = 0; i < namesOfFunctionQualifiedRead.size(); ++i) {
        //functionQualifiedReadProxy = new mtsFunctionQualifiedRead(providedInterface, namesOfFunctionQualifiedRead[i]);
        functionQualifiedReadProxy = new mtsFunctionQualifiedRead();
        success = requiredInterfaceProxy->AddFunction(namesOfFunctionQualifiedRead[i], *functionQualifiedReadProxy);
        //success &= FunctionQualifiedReadProxyMap.AddItem(namesOfFunctionQualifiedRead[i], functionQualifiedReadProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add qualified read function proxy: " << namesOfFunctionQualifiedRead[i] << std::endl;
            return false;
        }
    }

    // Create event handler proxies
    std::string eventName;

    // Create event handler proxies with CommandId set to zero which will be 
    // updated later by UpdateEventHandlerId().
    //
    // Note that all events created are disabled by default. Commands that are
    // actually bounded and used at the client will be enabled only by the
    // execution of UpdateEventHandlerId() method.

    // Create void event handler proxy
    mtsCommandVoidProxy * actualEventVoidCommandProxy = NULL;
    for (unsigned int i = 0; i < requiredInterfaceDescription.EventHandlersVoid.size(); ++i) {
        eventName = requiredInterfaceDescription.EventHandlersVoid[i].Name;
        //
        // TODO: Ice Proxy instance (per command proxy) should be passed to CommandVoidProxy object!!
        //
        actualEventVoidCommandProxy = new mtsCommandVoidProxy(0, (mtsComponentInterfaceProxyServer*) NULL, eventName);
        actualEventVoidCommandProxy->Disable();

        success = requiredInterfaceProxy->EventHandlersVoid.AddItem(eventName, actualEventVoidCommandProxy);
        //success &= EventHandlerVoidProxyMap.AddItem(eventName, actualEventVoidCommandProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add void event handler proxy: " << eventName << std::endl;
            return false;
        }
    }

    // Create write event handler proxy
    mtsCommandWriteProxy * actualEventWriteCommandProxy = NULL;    
    for (unsigned int i = 0; i < requiredInterfaceDescription.EventHandlersWrite.size(); ++i) {
        eventName = requiredInterfaceDescription.EventHandlersWrite[i].Name;
        //
        // TODO: Ice Proxy instance (per command proxy) should be passed to CommandVoidProxy object!!
        //
        actualEventWriteCommandProxy = new mtsCommandWriteProxy(0, /* TODO: UPDATE */ (mtsComponentInterfaceProxyServer*) NULL, eventName);
        actualEventWriteCommandProxy->Disable();

        success = requiredInterfaceProxy->EventHandlersWrite.AddItem(eventName, actualEventWriteCommandProxy);
        //success &= EventHandlerWriteProxyMap.AddItem(eventName, actualEventWriteCommandProxy);
        if (!success) {
            delete requiredInterfaceProxy;
            CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add write event handler proxy: " << eventName << std::endl;
            return false;
        }
    }

    // Add the required interface proxy to the component
    if (!AddRequiredInterface(requiredInterfaceName, requiredInterfaceProxy)) {
        CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: failed to add required interface proxy: " << requiredInterfaceName << std::endl;
        delete requiredInterfaceProxy;
        return false;
    }

    /*
    //
    // TODO: CHECK: Maybe, all the following codes are redundant because once a
    // proxy is created and added to LCM, it acts as if it were a plain component.
    //

    // Using AllocateResources(), get pointers which have been allocated for this 
    // required interface and are thread-safe to use.
    unsigned int userId;
    std::string userName = requiredInterfaceName + "Proxy";
    //
    // TODO: AllocateResources should resolve thread-safety issues in the connection between
    // multiple required interfaces and a provided interface
    // (e.g. serializer, stringstream buffer in serializer, ...)
    //
    //userId = providedInterface->AllocateResources(userName);

    /* Don't connect right now
    // Connect to the original device or task that provides allocated resources.
    requiredInterfaceProxy->ConnectTo(providedInterface);
    if (!requiredInterfaceProxy->BindCommandsAndEvents(userId)) {
        // return without adding the required interface proxy to the component
        CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: BindCommandsAndEvents failed: userName="
            << userName << ", userId=" << userId << std::endl;
        //
        // TODO: providedInterface->DeAllocateResources()????
        // TODO: requiredInterfaceProxy->DisconnectFrom()????
        //
        delete requiredInterfaceProxy;
        return false;
    }
    */

    CMN_LOG_CLASS_RUN_ERROR << "CreateRequiredInterfaceProxy: added required interface proxy: " << requiredInterfaceName << std::endl;

    return true;
}

bool mtsComponentProxy::RemoveRequiredInterfaceProxy(const std::string & requiredInterfaceProxyName)
{
    if (!RequiredInterfaces.FindItem(requiredInterfaceProxyName)) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveRequiredInterfaceProxy: cannot find required interface proxy: " << requiredInterfaceProxyName << std::endl;
        return false;
    }

    // Get a pointer to the provided interface proxy
    mtsRequiredInterface * requiredInterfaceProxy = RequiredInterfaces.GetItem(requiredInterfaceProxyName);
    if (!requiredInterfaceProxy) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveRequiredInterfaceProxy: This should not happen" << std::endl;
        return false;
    }

    // Remove the provided interface proxy from map
    if (!RequiredInterfaces.RemoveItem(requiredInterfaceProxyName)) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveRequiredInterfaceProxy: cannot remove required interface proxy: " << requiredInterfaceProxyName << std::endl;
        return false;
    }

    delete requiredInterfaceProxy;

    CMN_LOG_CLASS_RUN_VERBOSE << "RemoveRequiredInterfaceProxy: removed required interface proxy: " << requiredInterfaceProxyName << std::endl;
    return true;
}

//-----------------------------------------------------------------------------
//  Methods for Client Components
//-----------------------------------------------------------------------------
bool mtsComponentProxy::CreateProvidedInterfaceProxy(ProvidedInterfaceDescription & providedInterfaceDescription)
{
    const std::string providedInterfaceName = providedInterfaceDescription.ProvidedInterfaceName;

    // Create a local provided interface (a provided interface proxy) but it
    // is not immediately added to the component. The interface proxy can be 
    // added to the component proxy only after all proxy objects (command 
    // proxies and event proxies) are confirmed to be successfully created.
    mtsProvidedInterface * providedInterfaceProxy = new mtsDeviceInterface(providedInterfaceName, this);

    // Create command proxies according to the information about the original
    // provided interface. CommandId is initially set to zero and will be 
    // updated later by GetCommandId() which assigns command IDs to command
    // proxies.

    //
    //  TODO: GetCommandId() should be updated (or renamed)
    //

    // Note that argument prototypes in the interface description was serialized
    // so they should be deserialized in order to be used to recover orignial 
    // argument prototypes.
    std::string commandName;
    mtsGenericObject * argumentPrototype = NULL,
                     * argument1Prototype = NULL, 
                     * argument2Prototype = NULL;

    std::stringstream streamBuffer;
    cmnDeSerializer deserializer(streamBuffer);

    // Create void command proxies
    mtsCommandVoidProxy * newCommandVoid = NULL;
    CommandVoidVector::const_iterator itVoid = providedInterfaceDescription.CommandsVoid.begin();
    const CommandVoidVector::const_iterator itVoidEnd = providedInterfaceDescription.CommandsVoid.end();
    for (; itVoid != itVoidEnd; ++itVoid) {
        commandName = itVoid->Name;
        newCommandVoid = new mtsCommandVoidProxy(0, (mtsComponentInterfaceProxyClient*) NULL /* TODO: UPDATE THIS PROXY POINTER!!! */, commandName);
        if (!providedInterfaceProxy->GetCommandVoidMap().AddItem(commandName, newCommandVoid)) {
            // return without adding the provided interface proxy to the component
            delete newCommandVoid;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add void command proxy: " << commandName << std::endl;
            return false;
        }
    }

    // Create write command proxies
    mtsCommandWriteProxy * newCommandWrite = NULL;
    CommandWriteVector::const_iterator itWrite = providedInterfaceDescription.CommandsWrite.begin();
    const CommandWriteVector::const_iterator itWriteEnd = providedInterfaceDescription.CommandsWrite.end();
    for (; itWrite != itWriteEnd; ++itWrite) {
        commandName = itWrite->Name;
        newCommandWrite = new mtsCommandWriteProxy(0, (mtsComponentInterfaceProxyClient*) NULL /* TODO: UPDATE THIS PROXY POINTER!!! */, commandName);
        if (!providedInterfaceProxy->GetCommandWriteMap().AddItem(commandName, newCommandWrite)) {
            // return without adding the provided interface proxy to the component
            delete newCommandWrite;
            delete providedInterfaceProxy;            
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add write command proxy: " << commandName << std::endl;
            return false;
        }

        // argument deserialization
        streamBuffer.str("");
        streamBuffer << itWrite->ArgumentPrototypeSerialized;
        argumentPrototype = dynamic_cast<mtsGenericObject *>(deserializer.DeSerialize());
        if (!argumentPrototype) {
            // return without adding the provided interface proxy to the component
            delete argumentPrototype;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to create write command proxy: " << commandName << std::endl;
            return false;
        }
        newCommandWrite->SetArgumentPrototype(argumentPrototype);
    }

    // Create read command proxies
    mtsCommandReadProxy * newCommandRead = NULL;
    CommandReadVector::const_iterator itRead = providedInterfaceDescription.CommandsRead.begin();
    const CommandReadVector::const_iterator itReadEnd = providedInterfaceDescription.CommandsRead.end();
    for (; itRead != itReadEnd; ++itRead) {
        commandName = itRead->Name;
        newCommandRead = new mtsCommandReadProxy(0, (mtsComponentInterfaceProxyClient*) NULL, commandName);
        if (!providedInterfaceProxy->GetCommandReadMap().AddItem(commandName, newCommandRead)) {
            // return without adding the provided interface proxy to the component
            delete newCommandRead;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add read command proxy: " << commandName << std::endl;
            return false;
        }

        // argument deserialization
        streamBuffer.str("");
        streamBuffer << itRead->ArgumentPrototypeSerialized;
        argumentPrototype = dynamic_cast<mtsGenericObject *>(deserializer.DeSerialize());
        if (!argumentPrototype) {
            // return without adding the provided interface proxy to the component
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to create read command proxy: " << commandName << std::endl;
            return false;
        }
        newCommandRead->SetArgumentPrototype(argumentPrototype);
    }

    // Create qualified read command proxies
    mtsCommandQualifiedReadProxy * newCommandQualifiedRead = NULL;
    CommandQualifiedReadVector::const_iterator itQualifiedRead = providedInterfaceDescription.CommandsQualifiedRead.begin();
    const CommandQualifiedReadVector::const_iterator itQualifiedReadEnd = providedInterfaceDescription.CommandsQualifiedRead.end();
    for (; itQualifiedRead != itQualifiedReadEnd; ++itQualifiedRead) {
        commandName = itQualifiedRead->Name;
        newCommandQualifiedRead = new mtsCommandQualifiedReadProxy(0, (mtsComponentInterfaceProxyClient*) NULL, commandName);
        if (!providedInterfaceProxy->GetCommandQualifiedReadMap().AddItem(commandName, newCommandQualifiedRead)) {
            // return without adding the provided interface proxy to the component
            delete newCommandQualifiedRead;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add qualified read command proxy: " << commandName << std::endl;
            return false;
        }

        // argument1 deserialization
        streamBuffer.str("");
        streamBuffer << itQualifiedRead->Argument1PrototypeSerialized;
        argument1Prototype = dynamic_cast<mtsGenericObject *>(deserializer.DeSerialize());        
        // argument2 deserialization
        streamBuffer.str("");
        streamBuffer << itQualifiedRead->Argument2PrototypeSerialized;
        argument2Prototype = dynamic_cast<mtsGenericObject *>(deserializer.DeSerialize());        
        if (!argument1Prototype || !argument2Prototype) {
            // return without adding the provided interface proxy to the component
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to create qualified read command proxy: " << commandName << std::endl;
            return false;
        }
        newCommandQualifiedRead->SetArgumentPrototype(argument1Prototype, argument2Prototype);
    }

    // Create event generator proxies
    std::string eventName;

    // Create void event generator proxies
    mtsFunctionVoid * eventVoidGeneratorProxy = NULL;
    EventVoidVector::const_iterator itEventVoid = providedInterfaceDescription.EventsVoid.begin();
    const EventVoidVector::const_iterator itEventVoidEnd = providedInterfaceDescription.EventsVoid.end();
    for (; itEventVoid != itEventVoidEnd; ++itEventVoid) {
        eventName = itEventVoid->Name;
        eventVoidGeneratorProxy = new mtsFunctionVoid();
        //if (!EventVoidGeneratorProxyMap.AddItem(eventName, eventVoidGeneratorProxy)) {
        //    CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add void event proxy: " << eventName << std::endl;
        //    //
        //    // TODO: providedInterfaceProxy should be removed from 'component' because 
        //    // the integrity of the provided interface proxy is corrupted.
        //    //
        //    return false;
        //}
        
        if (!eventVoidGeneratorProxy->Bind(providedInterfaceProxy->AddEventVoid(eventName))) {
            // return without adding the provided interface proxy to the component
            delete eventVoidGeneratorProxy;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to bind with void event proxy: " << eventName << std::endl;
            return false;
        }
    }

    // Create write event generator proxies
    mtsFunctionWrite * eventWriteGeneratorProxy = NULL;
    mtsMulticastCommandWriteProxy * eventMulticastCommandProxy = NULL;

    EventWriteVector::const_iterator itEventWrite = providedInterfaceDescription.EventsWrite.begin();
    const EventWriteVector::const_iterator itEventWriteEnd = providedInterfaceDescription.EventsWrite.end();
    for (; itEventWrite != itEventWriteEnd; ++itEventWrite) {
        eventName = itEventWrite->Name;
        eventWriteGeneratorProxy = new mtsFunctionWrite();
        //if (!EventWriteGeneratorProxyMap.AddItem(eventName, eventWriteGeneratorProxy)) {
        //    CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add write event generator proxy: " << eventName << std::endl;
        //    //
        //    // TODO: providedInterfaceProxy should be removed from 'component' because 
        //    // the integrity of the provided interface proxy is corrupted.
        //    //
        //    return false;
        //}
        //
        eventMulticastCommandProxy = new mtsMulticastCommandWriteProxy(eventName);

        // event argument deserialization
        streamBuffer.str("");
        streamBuffer << itEventWrite->ArgumentPrototypeSerialized;
        argumentPrototype = dynamic_cast<mtsGenericObject *>(deserializer.DeSerialize());
        if (!argumentPrototype) {
            // return without adding the provided interface proxy to the component
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to create write event proxy: " << commandName << std::endl;
            return false;
        }
        eventMulticastCommandProxy->SetArgumentPrototype(argumentPrototype);

        if (!providedInterfaceProxy->AddEvent(eventName, eventMulticastCommandProxy)) {
            // return without adding the provided interface proxy to the component
            delete eventMulticastCommandProxy;
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add write event proxy: " << eventName << std::endl;
            return false;
        }
        if (!eventWriteGeneratorProxy->Bind(eventMulticastCommandProxy)) {
            // return without adding the provided interface proxy to the component
            delete providedInterfaceProxy;
            
            CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to bind with write event proxy: " << eventName << std::endl;
            return false;
        }
    }

    // Add the provided interface proxy to the component
    if (!ProvidedInterfaces.AddItem(providedInterfaceName, providedInterfaceProxy)) {
        CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: failed to add provided interface proxy: " << providedInterfaceName << std::endl;
        delete providedInterfaceProxy;
        return false;
    }

    CMN_LOG_CLASS_RUN_ERROR << "CreateProvidedInterfaceProxy: added provided interface proxy: " << providedInterfaceName << std::endl;

    return true;
}

bool mtsComponentProxy::RemoveProvidedInterfaceProxy(const std::string & providedInterfaceProxyName)
{
    if (!ProvidedInterfaces.FindItem(providedInterfaceProxyName)) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveProvidedInterfaceProxy: cannot find provided interface proxy: " << providedInterfaceProxyName << std::endl;
        return false;
    }

    // Get a pointer to the provided interface proxy
    mtsProvidedInterface * providedInterfaceProxy = ProvidedInterfaces.GetItem(providedInterfaceProxyName);
    if (!providedInterfaceProxy) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveProvidedInterfaceProxy: This should not happen" << std::endl;
        return false;
    }

    // Remove the provided interface proxy from map
    if (!ProvidedInterfaces.RemoveItem(providedInterfaceProxyName)) {
        CMN_LOG_CLASS_RUN_ERROR << "RemoveProvidedInterfaceProxy: cannot remove provided interface proxy: " << providedInterfaceProxyName << std::endl;
        return false;
    }

    delete providedInterfaceProxy;

    CMN_LOG_CLASS_RUN_VERBOSE << "RemoveProvidedInterfaceProxy: removed provided interface proxy: " << providedInterfaceProxyName << std::endl;
    return true;
}

bool mtsComponentProxy::CreateInterfaceProxyServer(const std::string & providedInterfaceProxyName,
                                                   std::string & adapterName, 
                                                   std::string & endpointAccessInfo,
                                                   std::string & communicatorId)
{
    mtsManagerLocal * managerLocal = mtsManagerLocal::GetInstance();

    const std::string adapterNameBase = "ComponentInterfaceServerAdapter";
    const std::string endpointInfoBase = "tcp -p ";
    const std::string portNumber = GetNewPortNumberAsString(ProvidedInterfaceProxies.size());
    const std::string endpointInfo = endpointInfoBase + portNumber;

    // Generate parameters to initialize server proxy    
    adapterName = adapterNameBase + "_" + providedInterfaceProxyName;    
    endpointAccessInfo = ":default -h " + managerLocal->GetIPAddress() + " " + "-p " + portNumber;
    communicatorId = ComponentInterfaceCommunicatorID;

    // Create an instance of mtsComponentInterfaceProxyServer
    mtsComponentInterfaceProxyServer * providedInterfaceProxy =
        new mtsComponentInterfaceProxyServer(adapterName, endpointInfo, communicatorId);

    // Add it to provided interface proxy object map
    if (!ProvidedInterfaceProxies.AddItem(providedInterfaceProxyName, providedInterfaceProxy)) {
        CMN_LOG_CLASS_RUN_ERROR << "CreateInterfaceProxyServer failed: "
            << "cannot register provided interface proxy: " << providedInterfaceProxyName << std::endl;
        return false;
    }

    // Run provided interface proxy (component interface proxy server)
    providedInterfaceProxy->Start(this);
    providedInterfaceProxy->GetLogger()->trace(
        "mtsComponentProxy", "Provided interface proxy starts: " + providedInterfaceProxyName);

    return true;
}

bool mtsComponentProxy::CreateInterfaceProxyClient(const std::string & requiredInterfaceProxyName)
{
    //
    // TODO
    //
    return false;
}

const std::string mtsComponentProxy::GetNewPortNumberAsString(const unsigned int id) const
{
    unsigned int newPortNumber = mtsProxyBaseCommon<mtsComponentProxy>::GetBasePortNumberForGlobalComponentManager() + (id * 5);

    std::stringstream newPortNumberAsString;
    newPortNumberAsString << newPortNumber;

    return newPortNumberAsString.str();
}
