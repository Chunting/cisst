/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: $

  Author(s):  Min Yang Jung, Anton Deguet
  Created on: 2010-09-01

  (C) Copyright 2010 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstCommon/cmnPortability.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsManagerLocal.h>

int main(int argc, char * argv[])
{
    // Set global component manager IP
    std::string globalComponentManagerIP;
    if (argc == 1) {
        globalComponentManagerIP = "localhost";
    } else if (argc == 2) {
        globalComponentManagerIP = argv[1];
    } else {
        return 1;
    }

    // Get local component manager instance
    mtsManagerLocal * ComponentManager;
    try {
        ComponentManager = mtsManagerLocal::GetInstance(globalComponentManagerIP, "ProcessCounter");
    } catch (...) {
        std::cout << "Failed to initialize local component manager" << std::endl;
        return 1;
    }

    // create the tasks, i.e. find the commands
    ComponentManager->CreateAll();
    // start the periodic Run
    ComponentManager->StartAll();
    
    while (true) {
        osaSleep(10 * cmn_ms);
    }
    
    // cleanup
    ComponentManager->KillAll();
    ComponentManager->Cleanup();

    return 0;
}

