//
//  openGLVisualiser.cpp
//  MedialAxisTransform
//
//  Created by Beau Johnston on 31/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#include "openGLVisualiser.h"

const int FRAMES_PER_SEC = 300;
clock_t clock_ticks = clock();
unsigned char * __dataSet;
int __imageWidth, __imageHeight, __imageDepth;
int __imageChannels = 4;
int cameraX = 0;
int cameraY = 0; 
int cameraZ = 0;

GLfloat degrees = 0.0;

void storeDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){
    __dataSet = new unsigned char[imageWidth*imageHeight*imageDepth*__imageChannels];
    
    for (int i = 0; i < __imageWidth*__imageHeight*__imageDepth*__imageChannels; i++) {
        __dataSet[i]=dataSet[i];
    }
    __imageWidth = imageWidth;
    __imageHeight = imageHeight;
    __imageDepth = imageDepth;
    return;
}

void createDataSetTexture(){
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, __imageWidth, __imageHeight, __imageDepth, 0, GL_RGBA,              GL_UNSIGNED_BYTE, __dataSet);
    
    delete[] __dataSet;
    
}

void Init(void)
{    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
    glClearColor(1.f, 1.f, 1.f, 0.f);
	
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	createDataSetTexture();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    //glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 5);
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1, 10.0);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glMatrixMode(GL_MODELVIEW);
    
	glLoadIdentity();
}

void glutPrint(float x, float y, float z, void * font, char* text, float r, float g, float b, float a) 
{ 
    if(!text || !strlen(text)) return; 
    bool blending = false; 
    if(glIsEnabled(GL_BLEND)) blending = true; 
    glEnable(GL_BLEND); 
    glColor3f(r,g,b); 
	glRasterPos3d(x, y, z);
    while (*text) { 
        glutBitmapCharacter(font, *text); 
        text++; 
    } 
    if(!blending) glDisable(GL_BLEND);	
}  

void display(void)
{	
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    //draw data here!
    //plotDataSet();
    glTranslated(cameraX, cameraY, cameraZ);

    for (int z = 0; z < __imageDepth; z++) {

        //glTranslated(0.f, 0.f, 1.0f);
        for (int y = 0; y < __imageHeight; y++) {
            glPushMatrix();
            glTranslated(cameraX, cameraY, cameraZ);
            

            glDrawPixels(__imageWidth, __imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, __dataSet);
            glPopMatrix();
        }
    }
	//glRotated(degrees, 1, 1, 1);
        
    //glFlush();
    //glutPrint(-0.5f, 1.4f, -1.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Y axis: W up, S down ", 0.0f, 0.5f, 1.0f, 0.0f);
    //clear colour
    //std::cout << "updated" << std::endl;
	//glColor3f(1.0f,1.0f,1.0f);
	glutSwapBuffers();
}

void mouseClick(int button, int state, int x, int y){
	if (button==GLUT_LEFT && state==GLUT_DOWN) {
        //on mouse click toggle paused
        std::cout << "mouse clicked at " << x << " and " << y << std::endl;
	}
}

void input(unsigned char character, int xx, int yy) {
	switch(character) {
		//case ' ' : changePaused(); break; 
		case 27  : exit(0);
        //up
        case 119 : std::cout << "up pressed" << std::endl; cameraY+=0.1; break;
        //down
        case 115 : std::cout << "down pressed" << std::endl; cameraY -=0.1; break;
        //left
        case 97 : std::cout << "left pressed" << std::endl; cameraX -=0.1; break;
        //right
        case 100 : std::cout << "right pressed" << std::endl; cameraX +=0.1; break;
        //zoom in
        case 114 : std::cout << "zoom in" << std::endl; cameraZ +=0.1; break;
        //zoom out
        case 102 : std::cout << "zoom out" << std::endl; cameraZ -=0.1; break;

        default: std::cout << "unknown key pressed : " << (int)character << std::endl;
	}
}

void increaseRotation(void){
	degrees += 0.25;
	if (degrees > 360) {
		degrees -= 360;
	}
}

void wait(float seconds)
{
	clock_t endwait;
	endwait = clock_t(clock () + seconds * CLOCKS_PER_SEC);
	while (clock() < endwait) {}
}

void update(void)
{	
    // Wait and update the clock
	clock_t clocks_elapsed = clock() - clock_ticks; 
	if ((float) clocks_elapsed < (float) CLOCKS_PER_SEC / (float) FRAMES_PER_SEC)
		wait(((float)CLOCKS_PER_SEC / (float) FRAMES_PER_SEC - (float) clocks_elapsed)/(float) CLOCKS_PER_SEC);
	//clock_ticks = clock();
	//increaseRotation();

    //move data plot?
    
    
	glutPostRedisplay();
}

void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth)
{
    storeDataSet(dataSet, imageWidth, imageHeight, imageDepth);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition (0, 0);
	glutCreateWindow("Data Visualiser");
	Init();
    glutMouseFunc(&mouseClick);
    glutKeyboardFunc(input);
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMainLoop();
}
