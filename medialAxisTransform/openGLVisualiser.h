//
//  openGLVisualiser.h
//  MedialAxisTransform
//
//  Created by Beau Johnston on 31/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

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

void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth);

//#define RAYCASTING
//#define MARCHING_CUBES
#define PLANE_PLOT

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
#endif

#endif