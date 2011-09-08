/*
 *  openGLVisualiser.h
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 17/06/11.
 *  Copyright (C) 2011 by Beau Johnston.
 *
 *  Please email me if you have any comments, suggestions or advice:
 *                              beau@inbeta.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */


#ifndef OPENCL_UTILITIES
#define OPENCL_UTILITIES

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "RGBAUtilities.h"
#include "FileHandler.h"


// openCL is dependent on specific libraries, depending on the OS
#if defined (__APPLE__)  && defined (__MACH__)
    //macOSX openCL Framework
    #include <OpenCL/opencl.h>
#else
    //it must be for a LinuxBox
    //linux openCL Library
    //#include <CL/oclUtils.h>
    #include <CL/opencl.h>
#endif

#define FATAL(msg)\
do {\
fprintf(stderr,"FATAL [%s:%d]:%s:%s\n", __FILE__, __LINE__, msg, strerror(errno)); \
assert(0); \
} while(0)

#define SRC 1
#define DST 2
#define byte unsigned char

class OpenCLUtilities //Sample Class for the C++ Tutorial 
{
private:
    FileHandler *fileHandler;
    RGBAUtilities *rgbaUtilities;
    
public: 
    OpenCLUtilities();
    ~OpenCLUtilities();
    char *print_cl_errstring(cl_int err);
    cl_bool there_was_an_error(cl_int err);
    void getGPUUnitSupportedImageFormats(cl_context context);
    cl_bool doesGPUSupportImageObjects(cl_device_id device_id);
    char *load_program_source(const char *filename);
    cl_bool cleanupAndKill();
    cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height, cl_image_format &format);
    cl_mem LoadStackOfImages(cl_context context, char *fileName, int &width, int &height, int &depth, cl_image_format &format);
    
    bool SaveImage(char *fileName, uint8 *buffer, int width, int height);
    cl_mem FreeImageLoadImage(cl_context context, char *fileName, int &width, int &height, cl_image_format &format);
    bool FreeImageSaveImage(char *fileName, char *buffer, int width, int height);
    size_t RoundUp(int groupSize, int globalSize);
    size_t getImageHeight();
    size_t getImageWidth();
    size_t getImageSize();
    size_t getImageRowPitch();
    size_t getImageSlicePitch();
};




#endif
