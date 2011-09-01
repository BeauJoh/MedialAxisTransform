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
int cameraZ = -3.0f;
GLuint tex;

GLfloat degrees = 0.5;

int cameraXDegrees = 0;
int cameraYDegrees = 0;
int cameraZDegrees = 0;

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

// these define a square in the X-Y plane clockwise order from the lower left,
GLdouble verts[4][3] = { { -1.0, -1.0, 0.0}, {-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}, {1.0, -1.0, 0.0} };
GLdouble centervert[3] = { 0.0, 0.0, 0.0 };

void createDataSetTexture(){

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
//    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NICEST);
//    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, __imageWidth, __imageHeight, __imageDepth, 0, GL_RGBA,              GL_UNSIGNED_BYTE, __dataSet);
    
    //glEnable(GL_TEXTURE_3D);
    //delete[] __dataSet;
}

void Init(void)
{    
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_DEPTH_TEST);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor4d(0.0, 0.0, 0.0, 1.0);
    glPointSize(3.0);
    
    glShadeModel(GL_FLAT);

    glViewport (0, 0, (GLsizei) 1000, (GLsizei) 1000);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(90, 4.0/3.0, 0.1, 10.0);

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glShadeModel(GL_SMOOTH);	
	//glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    
//    glDepthMask(GL_TRUE);

    //glEnable(GL_TEXTURE_BINDING_3D);
    //glEnable(GL_TEXTURE_GEN_S);
    //glEnable(GL_TEXTURE_GEN_T);
    //glEnable(GL_TEXTURE_GEN_R);

    //glViewport(0, 0, -1, -1);
    //glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 5);
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1, 10.0);

    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
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
    float y, z;
	int x;
    
    glBindTexture(GL_TEXTURE_3D, tex);   // choose the texture to use.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

    
    //draw data here!
    //plotDataSet();
    glTranslated(cameraX, cameraY, cameraZ);

    glRotatef(cameraXDegrees,1.0f,0.0f,0.0f);		// Rotate On The X Axis
    glRotatef(cameraYDegrees,0.0f,1.0f,0.0f);		// Rotate On The Y Axis
    
    centervert[2] = sin(3000.0) + 1.0;
    
	// draw the pyramid with the fluctuating apex
	// turn on 3d texturing if its not already
	glEnable(GL_TEXTURE_3D);
	glBegin(GL_TRIANGLES);
    // texture coordinates are always specified before the vertex they apply to.
    for (x = 0; x <= 3; x++) {
        glColor4d(1.0, 0.0, 0.0, 1.0);
        glTexCoord3d(centervert[0], centervert[1], centervert[2]);
        //glTexCoord3d(centervert[0], centervert[1], 2.0);			// texture stretches rather than glides over the surface with this
        glVertex3d(centervert[0], centervert[1], centervert[2]);
        
        glTexCoord3d(verts[x][0], verts[x][1], verts[x][2]);
        glVertex3d(verts[x][0], verts[x][1], verts[x][2]);
        
        glTexCoord3d(verts[(x+1)%4][0], verts[(x+1)%4][1], verts[(x+1)%4][2]);
        glVertex3d(verts[(x+1)%4][0], verts[(x+1)%4][1], verts[(x+1)%4][2]);
        glColor4d(0.0, 0.0, 0.0, 1.0);

    }
	glEnd();
    
	// we don't want the lines and points textured, so disable 3d texturing for a bit
	glDisable(GL_TEXTURE_3D);
	glBegin(GL_LINES);
    // draw the grids
    for (z = 0; z < 2.0; z += 0.5) {
        for (y = -1.0; y <= 1.0; y += 0.5) {
            glVertex3d(y, -1.0, z);	
            glVertex3d(y, 1.0, z);
        }
        for (y = -1.0; y <= 1.0; y += 0.5) {
            glVertex3d(-1.0, y, z);	
            glVertex3d(1.0, y, z);
        }
    }
    // draw the wire frame size of the pyramid
    for (x = 0; x < 4; x++) {
        glVertex3d(centervert[0], centervert[1], centervert[2]);
        glVertex3d(verts[x][0], verts[x][1], verts[x][2]);
    }
	glEnd();
	// draw the apex point
	glBegin(GL_POINTS);
    glVertex3d(centervert[0], centervert[1], centervert[2]);
	glEnd();
    
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
        case 119 : std::cout << "up pressed" << std::endl; cameraY+=1; break;
        //down
        case 115 : std::cout << "down pressed" << std::endl; cameraY -=1; break;
        //left
        case 97 : std::cout << "left pressed" << std::endl; cameraX -=1; break;
        //right
        case 100 : std::cout << "right pressed" << std::endl; cameraX +=1; break;
        //zoom in
        case 114 : std::cout << "zoom in" << std::endl; cameraZ +=1; break;
        //zoom out
        case 102 : std::cout << "zoom out" << std::endl; cameraZ -=1; break;
            
        case 106 : std::cout << "rotate up" << std::endl; cameraYDegrees +=1; break;

        case 108 : std::cout << "rotate down" << std::endl; cameraYDegrees -=1; break;
            
        case 105 : std::cout << "rotate left" << std::endl; cameraXDegrees -=1; break;

        case 107 : std::cout << "rotate right" << std::endl; cameraXDegrees +=1; break;


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
    
    createDataSetTexture();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
	Init();
    glutMouseFunc(&mouseClick);
    glutKeyboardFunc(input);
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMainLoop();
}
