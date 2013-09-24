/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s):  Simon Leonard
  Created on: 2009-11-11

  (C) Copyright 2009-2013 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _robFunction_h
#define _robFunction_h

#include <cisstRobot/robExport.h>

class CISST_EXPORT robFunction{

 protected:

    double t1;
    double t2;

 public:

    robFunction( void );
    robFunction( double startTime, double stopTime );

    void Set( double startTime, double stopTime );
    virtual double& StartTime( void );
    virtual double& StopTime( void );
    virtual double Duration( void ) const;

};

#endif // _robFunction_h
