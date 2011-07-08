/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$
  
  Author(s):  Balazs Vagvolgyi
  Created on: 2007 

  (C) Copyright 2006-2007 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _svlConverters_h
#define _svlConverters_h

#include <cisstStereoVision/svlStreamDefs.h>

void ConvertImage(svlSampleImageBase* input, svlSampleImageBase* output, int param = 0, unsigned int threads = 1, unsigned int threadid = 0);

void RGB24toGray8(unsigned char* input, unsigned char* output, const unsigned int pixelcount, bool accurate = false, bool bgr = false);
void RGB24toGray16(unsigned char* input, unsigned short* output, const unsigned int pixelcount, bool accurate = false, bool bgr = false);
void Gray8toRGB24(unsigned char* input, unsigned char* output, const unsigned int pixelcount);
void Gray8toGray16(unsigned char* input, unsigned short* output, const unsigned int pixelcount);
void Gray16toRGB24(unsigned short* input, unsigned char* output, unsigned int pixelcount, const unsigned int shiftdown = 0);
void Gray16toGray8(unsigned short* input, unsigned char* output, unsigned int pixelcount, const unsigned int shiftdown = 0);
void int32toRGB24(int* input, unsigned char* output, const unsigned int pixelcount, const int maxinputvalue = -1);
void int32toGray8(int* input, unsigned char* output, const unsigned int pixelcount, const int maxinputvalue = -1);
void float32toRGB24(float* input, unsigned char* output, const unsigned int pixelcount, const float scalingratio = 1.0f);
void float32toGray8(float* input, unsigned char* output, const unsigned int pixelcount, const float scalingratio = 1.0f);
void float32toGray16(float* input, unsigned short* output, const unsigned int pixelcount, const float scalingratio = 1.0f);
void RGB24toYUV24(unsigned char* input, unsigned char* output, const unsigned int pixelcount, bool ch1 = true, bool ch2 = true, bool ch3 = true);
void RGB24toHSV24(unsigned char* input, unsigned char* output, const unsigned int pixelcount, bool ch1 = true, bool ch2 = true, bool ch3 = true);
void RGB24toHSL24(unsigned char* input, unsigned char* output, const unsigned int pixelcount, bool ch1 = true, bool ch2 = true, bool ch3 = true);

#endif // _svlConverters_h
