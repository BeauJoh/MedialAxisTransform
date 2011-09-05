//
//  openGLVisualiser.h
//  MedialAxisTransform
//
//  Created by Beau Johnston on 31/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#ifndef MedialAxisTransform_openGLVisualiser_h
#define MedialAxisTransform_openGLVisualiser_h

#ifdef __APPLE__ & __MACH__
//macOSX openGL & GLUT Framework
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
//it must be for UNIX
#include <GL/glut.h>
#include <GL/gl.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <fstream>
#include <iostream>

void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);

#endif