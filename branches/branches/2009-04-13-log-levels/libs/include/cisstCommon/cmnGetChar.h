/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$
  
  Author(s):  Anton Deguet, Balazs Vagvolgyi
  Created on: 2009-03-26

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/


#ifndef _cmnGetChar_h
#define _cmnGetChar_h

#include <cisstCommon/cmnPortability.h>

// Always the last cisst include
#include <cisstCommon/cmnExport.h>



/*! Class used to setup a non blocking terminal used to get characters
  without any flushing, i.e. carriage return or new line */
class CISST_EXPORT cmnGetCharEnvironment
{
    /*! Internals that are OS-dependent */
#if (CISST_OS == CISST_LINUX) || (CISST_OS == CISST_DARWIN) || (CISST_OS == CISST_SOLARIS) || (CISST_OS == CISST_LINUX_RTAI)
    enum {INTERNALS_SIZE = 124};
#endif // CISST_LINUX || CISST_DARWIN ||CISST_SOLARIS || CISST_RTAI
#if (CISST_OS == CISST_WINDOWS)
    enum {INTERNALS_SIZE = 1};
#endif // CISST_WINDOWS

    char Internals[INTERNALS_SIZE];

    /*! Return the size of the actual object used by the OS.  This is
      used for testing only. */ 
    static unsigned int SizeOfInternals(void);
    friend class cmnGetCharEnvironmentTest;
    
    /*! Keep a flag status to make sure the environment is activated only once */
    bool Activated;

 public:

    cmnGetCharEnvironment(void);
    ~cmnGetCharEnvironment(void);

    /*! Activate non blocking */
    bool Activate(void);

    /*! De-activate non blocking, i.e. restore original terminal settings */
    bool DeActivate(void);

    /*! Get a single character without having to type "return" */
    int GetChar(void);
};

/*! Global function to get a single character without having to type
  "return".  This function creates and activates a
  cmnGetCharEnvironment and deletes it at each call.  If you need to
  get multiple characters you should probably use the
  cmnGetCharEnvironment class. */
int CISST_EXPORT cmnGetChar(void);


#endif // _cmnGetChar_h

