/*
 *  openGLVisualiser.h
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 31/08/11.
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


#ifndef MedialAxisTransform_openGLVisualiser_h
#define MedialAxisTransform_openGLVisualiser_h

#if defined(__APPLE__) && defined(__MACH__)
    //macOSX openGL & GLUT Framework
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
#else
    //it must be for NIX
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

//uncomment required def to change visualiser
#define MARCHING_CUBES
//#define PLANE_PLOT
//#define RAYCASTING


#ifdef MARCHING_CUBES
    void LoadTexture(unsigned char * data, int depth);
    void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);   
    void DisplayUsability(void);
    void Init(void);
    void Display(void);
    void MouseClick(int button, int state, int x, int y);
    void Input(unsigned char character, int xx, int yy);
    void IncreaseRotation(void);
    void Wait(float seconds);
    void Update(void);
    void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);
#endif

#ifdef PLANE_PLOT
    void LoadTexture(unsigned char * data, int depth);
    void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);   
    void DisplayUsability(void);
    void Init(void);
    void Display(void);
    void MouseClick(int button, int state, int x, int y);
    void Input(unsigned char character, int xx, int yy);
    void IncreaseRotation(void);
    void Wait(float seconds);
    void Update(void);
    void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);
#endif

#ifdef RAYCASTING
    void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);
#endif

#endif