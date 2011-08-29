//
//  openCLUtilities.h
//  openCLImageLoad
//
//  Created by Beau Johnston on 17/06/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

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


// Special automatic include statement, 
// openCL is dependent on specific libraries, depending on the OS
#if defined (__APPLE__)  && defined (__MACH__)
//macOSX openCL Framework
#include <OpenCL/opencl.h>
#else
//it must be for Tukey
//linux openCL Library
//#include <CL/oclUtils.h>
#include <CL/opencl.h>
//#include <tiff.h>
#endif

#define FATAL(msg)\
do {\
fprintf(stderr,"FATAL [%s:%d]:%s:%s\n", __FILE__, __LINE__, msg, strerror(errno)); \
assert(0); \
} while(0)

#define SRC 1
#define DST 2
#define byte unsigned char


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
size_t getImageSlicePitch();

#endif
