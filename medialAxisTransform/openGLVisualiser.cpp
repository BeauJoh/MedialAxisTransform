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
int cameraZ = 0.0f;
GLuint * textures;

GLfloat degrees = 0.5;

float sliceDepth = 0.3;
float sliceScalingFactor = 14.0f;

int cameraXDegrees = 0;
int cameraYDegrees = 0;
int cameraZDegrees = 0;

void LoadTexture(unsigned char * data, int depth)
{
    // allocate a texture name
    glGenTextures( 1, &textures[depth]);
    
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, textures[depth] );
    
    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NICEST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, __imageWidth, __imageHeight, 0, GL_RGBA,              GL_UNSIGNED_BYTE, __dataSet);
    
    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, __imageWidth, __imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, __dataSet );
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, __imageWidth, __imageHeight, 0, GL_RGBA,              GL_UNSIGNED_BYTE, __dataSet);
}

void storeDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){    
    __imageWidth = imageWidth;
    __imageHeight = imageHeight;
    __imageDepth = imageDepth;
    
    textures = new GLuint[__imageDepth];
    
    __dataSet = new unsigned char[__imageWidth*__imageHeight*4];
    
    for (int j = 0; j < imageDepth; j++) {
        for (int i = 0; i < __imageWidth*__imageHeight*4; i++) {
            __dataSet[i]=dataSet[i+(j*__imageWidth*__imageHeight*4)];
        }
        
        LoadTexture(__dataSet, j);
    }
    
    return;
}

void Init(void)
{    
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_DEPTH_TEST);
    
    glClearColor(0, 0.5, 1, 1);

    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //glClearColor(1.0, 1.0, 1.0, 1.0);
	//glColor4d(0.0, 0.0, 0.0, 1.0);
    //glPointSize(3.0);
    
    glShadeModel(GL_SMOOTH);
    
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(90, 3.0/3.0, 0.1, 100.0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glDepthFunc(GL_LEQUAL);
    
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
    glShadeModel(GL_FLAT);
    
    glClear(GL_COLOR_BUFFER_BIT );
    	
    glLoadIdentity();
    
    //glutPrint(0, 0, 1, GLUT_BITMAP_TIMES_ROMAN_24, "bananas", 1, 0.5, 0, 0.1);
    glTranslated(cameraX, cameraY, cameraZ);
    glTranslated(0.0f, 0.0f, -(__imageDepth+2));
    
    glRotatef(cameraXDegrees,1.0f,0.0f,0.0f);		// Rotate On The X Axis
    glRotatef(cameraYDegrees,0.0f,1.0f,0.0f);		// Rotate On The Y Axis

	glEnable(GL_TEXTURE_2D);
    // texture coordinates are always specified before the vertex they apply to.
    for (int i = 0; i < __imageDepth; i++) {
        
        glBindTexture(GL_TEXTURE_2D, textures[i]);   // choose the texture to use.
        
        glColor4f(1, .5, 0, 1);
        
        glBegin( GL_QUADS );
        glTexCoord2d(0.0,0.0); glVertex3d(0.0*sliceScalingFactor,0.0*sliceScalingFactor,i*sliceDepth);
        glTexCoord2d(1.0,0.0); glVertex3d(1.0*sliceScalingFactor,0.0*sliceScalingFactor,i*sliceDepth);
        glTexCoord2d(1.0,1.0); glVertex3d(1.0*sliceScalingFactor,1.0*sliceScalingFactor,i*sliceDepth);
        glTexCoord2d(0.0,1.0); glVertex3d(0.0*sliceScalingFactor,1.0*sliceScalingFactor,i*sliceDepth);
        glEnd();
        
    }
    
	// we don't want the lines and points textured, so disable 3d texturing for a bit
	glDisable(GL_TEXTURE_2D);
    
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
		case 27  : exit(0); break;
            // q or esc to quit    
        case 113 : exit(0); break;
            //up
        case 119 : std::cout << "up pressed" << std::endl; cameraY-=1; break;
            //down
        case 115 : std::cout << "down pressed" << std::endl; cameraY +=1; break;
            //left
        case 97 : std::cout << "left pressed" << std::endl; cameraX +=1; break;
            //right
        case 100 : std::cout << "right pressed" << std::endl; cameraX -=1; break;
            //zoom in
        case 114 : std::cout << "zoom in" << std::endl; cameraZ +=1; break;
            //zoom out
        case 102 : std::cout << "zoom out" << std::endl; cameraZ -=1; break;
            
        case 106 : std::cout << "rotate up" << std::endl; cameraYDegrees +=1; break;
            
        case 108 : std::cout << "rotate down" << std::endl; cameraYDegrees -=1; break;
            
        case 105 : std::cout << "rotate left" << std::endl; cameraXDegrees +=1; break;
            
        case 107 : std::cout << "rotate right" << std::endl; cameraXDegrees -=1; break;
            
            
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
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition (0, 0);
	glutCreateWindow("Data Visualiser");
    
    Init();

    storeDataSet(dataSet, imageWidth, imageHeight, imageDepth);
    
    glutMouseFunc(&mouseClick);
    glutKeyboardFunc(input);
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMainLoop();
}
