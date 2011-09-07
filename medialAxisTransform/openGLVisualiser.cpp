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

GLfloat degrees = 0.5;

float sliceDepth = 0.3;
float sliceScalingFactor = 14.0f;

int cameraXDegrees = 0;
int cameraYDegrees = 0;
int cameraZDegrees = 0;

float colour[4] = {1.0f,1.0f,0.75f,0.25};
float clearcolour[4] = {1.0f,1.0f,1.0f,1.0f};



//uncomment this and try to get it raycasting with 3D texture data
#ifdef RAYCASTING
const float kDefaultDistance = 2.25;
const float kMaxDistance = 40;

int ClearColor[3] = {0,0,0};
char* VolumeFilename;
bool Perspective = false;
float Distance = kDefaultDistance;
int Azimuth = 110;
int Elevation = 15;
int LUTCenter = 128;
int LUTWidth = 255;
int LUTindex = 0;
int showGradient = 0;
float stepSize = 0.01;
float edgeThresh = 0.05;
float edgeExp = 0.5;
float DitherRay = 1.0;
float boundExp = 0.0;    //Contribution of boundary enhancement calculated opacity and scaling exponent
int WINDOW_WIDTH,WINDOW_HEIGHT;
GLuint TransferTexture,gradientTexture3D,intensityTexture3D,finalImage,renderBuffer, frameBuffer,backFaceBuffer, glslprogram;

float ProportionalScale[3] = {1,1,1};


GLuint Load3DTextures(GLuint intensityVolume){// Load 3D data                                                                 
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1, &intensityVolume);
    glBindTexture(GL_TEXTURE_3D, intensityVolume);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//?
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//?
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);//?
    
    glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, __imageWidth, __imageHeight, __imageDepth,0, GL_RGBA, GL_UNSIGNED_BYTE, &__dataSet);
    
}


void enableRenderbuffers(void){
    glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, frameBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderBuffer);
}

void disableRenderBuffers(void){
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void drawVertex(float x,float y,float z){
    glColor3f(x,y,z);
    glMultiTexCoord3f(GL_TEXTURE1, x, y, z);
    glVertex3f(x,y,z);
}

void drawQuads(float x, float y, float z)
//x,y,z typically 1.
// useful for clipping
// If x=0.5 then only left side of texture drawn
// If y=0.5 then only posterior side of texture drawn
// If z=0.5 then only inferior side of texture drawn
{
    glBegin(GL_QUADS);
    //* Back side
    glNormal3f(0.0, 0.0, -1.0);
    drawVertex(0.0, 0.0, 0.0);
    drawVertex(0.0, y, 0.0);
    drawVertex(x, y, 0.0);
    drawVertex(x, 0.0, 0.0);
    //* Front side
    glNormal3f(0.0, 0.0, 1.0);
    drawVertex(0.0, 0.0, z);
    drawVertex(x, 0.0, z);
    drawVertex(x, y, z);
    drawVertex(0.0, y, z);
    //* Top side
    glNormal3f(0.0, 1.0, 0.0);
    drawVertex(0.0, y, 0.0);
    drawVertex(0.0, y, z);
    drawVertex(x, y, z);
    drawVertex(x, y, 0.0);
    //* Bottom side
    glNormal3f(0.0, -1.0, 0.0);
    drawVertex(0.0, 0.0, 0.0);
    drawVertex(x, 0.0, 0.0);
    drawVertex(x, 0.0, z);
    drawVertex(0.0, 0.0, z);
    //* Left side
    glNormal3f(-1.0, 0.0, 0.0);
    drawVertex(0.0, 0.0, 0.0);
    drawVertex(0.0, 0.0, z);
    drawVertex(0.0, y, z);
    drawVertex(0.0, y, 0.0);
    //* Right side
    glNormal3f(1.0, 0.0, 0.0);
    drawVertex(x, 0.0, 0.0);
    drawVertex(x, y, 0.0);
    drawVertex(x, y, z);
    drawVertex(x, 0.0, z);
    glEnd();
}


char* CheckForErrors(GLhandleARB glObject){
    GLint blen,slen;
    char* InfoLog;
    
    glGetObjectParameterivARB(glObject, GL_OBJECT_INFO_LOG_LENGTH_ARB, &blen);
    if (blen > 1){
        //        InfoLog = malloc(blen*sizeof(int));
        //        glGetInfoLogARB(glObject, blen, slen, InfoLog);
        //
        //        Result:= PAnsiChar(InfoLog);
        //        Dispose(InfoLog);
        std::cout << "Something bad happened!" << std::endl;
    }
}

std::string LoadStr (std::string lFilename){
    std::ifstream myFile;
    
    std::string text;
    std::string result = "";
    myFile.open(lFilename.c_str());
    
    if (!myFile.is_open()){
        std::cout << "Unable to find " << lFilename << std::endl;
        exit(-1);
    }
    
    while (myFile.good()){
        getline(myFile, text);
        result = result+text;
    }
    myFile.close();
    return result;
}

bool fileexists(std::string filename){
    std::ifstream instream;
    
    instream.open(filename.c_str());
    if (!instream.is_open()) {
        return false;
    }
    instream.close();
    return true;
}

std::string extractfilepath(std::string handedInString){
    return handedInString.substr(0, handedInString.find_last_of('/')+1);
}


unsigned long getFileLength(std::ifstream& file)
{
    if(!file.good()) return 0;
    
    unsigned long pos=file.tellg();
    file.seekg(0,std::ios::end);
    unsigned long len = file.tellg();
    file.seekg(std::ios::beg);
    
    return len;
}

int loadshader(char* filename, GLchar** ShaderSource)
{
    unsigned long len;
    std::ifstream file;
    file.open(filename, std::ios::in); // opens as ASCII!
    if(!file) return -1;
    
    len = getFileLength(file);
    
    if (len==0) return -2;   // Error: Empty File 
    
    *ShaderSource = (char*) new char[len+1];
    if (*ShaderSource == 0) return -3;   // can't reserve memory
    
    // len isn't always strlen cause some characters are stripped in ascii read...
    // it is important to 0-terminate the real length later, len is just max possible value... 
    *ShaderSource[len] = 0; 
    
    unsigned int i=0;
    while (file.good())
    {
        *ShaderSource[i] = file.get();       // get character from file.
        if (!file.eof())
            i++;
    }
    
    *ShaderSource[i] = 0;  // 0-terminate it at the correct position
    
    file.close();
    
    return 0; // No Error
}

int unloadshader(GLubyte** ShaderSource)
{
    if (*ShaderSource != 0)
        delete[] *ShaderSource;
    *ShaderSource = 0;
}


int initShaderWithFile(std::string vertname, std::string fragname) {
    
    GLhandleARB my_program, my_vertex_shader,my_fragment_shader;
    int Vcompiled,Fcompiled,FShaderLength,VShaderLength;
    std::string fname,vname,my_fragment_shader_source, my_vertex_shader_source,lErrors;
    // Create Shader And Program Objects
    int result = 0;
    Vcompiled = 0;
    Fcompiled = 0;
    fname = fragname;
    vname = vertname;
    if (!fileexists(fname)){
        fname = extractfilepath(fname);
        vname = vertname;
    }
    if (!fileexists(vname)){
        vname = extractfilepath(vname);
    }
    if ((!(fileexists(fname))) || (!(fileexists(vname)))) {
        std::cout << "Unable to find GLSL shaders " << fname << " " << vname << std::endl;
    }
    
//    char** tmpArray = new char*[1];
//    tmpArray[0] = (char*)my_fragment_shader_source.c_str();
//    char** tmpArray2 = new char*[1];
//    tmpArray2[0] = (char*)my_vertex_shader_source.c_str();
//    
//    loadshader((char*)fname.c_str(), (GLchar**)tmpArray);
//    loadshader((char*)vname.c_str(), (GLchar**)tmpArray2);

    //loadshader((char*)fname.c_str(), (GLchar**)my_fragment_shader_source.c_str());
    //loadshader((char*)vname.c_str(), (GLchar**)my_vertex_shader_source.c_str());
    my_fragment_shader_source = LoadStr( fname);
    my_vertex_shader_source = LoadStr( vname);
    my_program = glCreateProgramObjectARB();
    my_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    my_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    // Load Shader Sources
    FShaderLength = (int)my_fragment_shader_source.length();
    VShaderLength = (int)my_vertex_shader_source.length();
    const char * tmp1 = (const char *)my_vertex_shader_source.c_str();
    const char * tmp2 = (const char *)my_fragment_shader_source.c_str();

    glShaderSourceARB(my_vertex_shader, 1, &tmp1, NULL);
    glShaderSourceARB(my_fragment_shader, 1, &tmp2, NULL);
    // Compile The Shaders
    glCompileShaderARB(my_vertex_shader);
    glGetObjectParameterivARB(my_vertex_shader,GL_OBJECT_COMPILE_STATUS_ARB, &Vcompiled);
    glCompileShaderARB(my_fragment_shader);
    glGetObjectParameterivARB(my_fragment_shader,GL_OBJECT_COMPILE_STATUS_ARB, &Fcompiled);
    // Attach The Shader Objects To The Program Object
    glAttachObjectARB(my_program, my_vertex_shader);
    glAttachObjectARB(my_program, my_fragment_shader);
    // Link The Program Object
    glLinkProgramARB(my_program);
    if ((Vcompiled!=1) || (Fcompiled!=1)){
        //        lErrors := CheckForErrors(my_program);
        //        if lErrors <> '' then
        //            showDebug('GLSL shader error '+lErrors);
        //        end;
        //        result := my_program ;
        std::cout << "shader has errors!" << std::endl;
    }
    
    //result = my_program
    int i = 0;
    memcpy(&i, &my_program, sizeof(my_program));
    result = i;
    
    if (result == 0){
        std::cout << "Unable to load or compile shader" << std::endl;
    }
    
    return result;
}

void reshapeOrtho(int w, int h){
    if (h == 0){
        h = 1;
    }
    glViewport(0, 0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1.0);
    glMatrixMode(GL_MODELVIEW);//?
}

void resize(int w, int h){
    int whratio;
    int scale;
    
    if (h == 0){
        h = 1;
    }
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (Perspective){
        gluPerspective(40.0, w/h, 0.01, kMaxDistance - Distance);
    }else{ 
        if (Distance == 0){
            scale = 1;
        }else{
            scale = 1/ abs(round(kDefaultDistance/(Distance+1.0))) ;
        }
    }
    whratio = w/h;
    glOrtho(whratio*-0.5*scale,whratio*0.5*scale,-0.5*scale,0.5*scale, 0.01, kMaxDistance);
    
    glMatrixMode(GL_MODELVIEW);//?
}

void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){    
    __imageWidth = imageWidth;
    __imageHeight = imageHeight;
    __imageDepth = imageDepth;
    
    __dataSet = new unsigned char[__imageWidth*__imageHeight*__imageDepth*4];
    
    for (int i = 0; i < __imageWidth*__imageHeight*__imageDepth*4; i++) {
        __dataSet[i]=dataSet[i];
    }
    return;
}

//unsigned char * LUTrgba (int lIndex, int lInc){
//    const int k = 155;
//
//    unsigned char* result = new unsigned char [5];
//
//    switch(lIndex){
//        case 1 : result = {255,0,0,k,lInc}; break;//red
//        case 2 : result = LUTpt (0,255,0,k,lInc); break;//green
//        case 3 : result = LUTpt (0,0,255,k,lInc); break;//blue
//        case 4 : result = LUTpt (255,0,255,k,lInc); break;//violet
//        case 5 : result = LUTpt (255,255,0,k,lInc); break;//yellow
//        case 6 : result = LUTpt (0,255,255,k,lInc); break;//cyan
//        case 7 : result = LUTpt (243,201,151,151,lInc); break;//brain
//        default: result = LUTpt (255,255,255,255,lInc); break;//grayscale
//
//    }
//    return result;
//}

//procedure  LoadLUT ( lIndex,LUTCenter,LUTWidth: integer; var lLUT: tVolRGBA);
////Index: 0=gray,1=red,2=green,3=blue
//var
//lMax,lMin,lSlope: single;
//lInc: integer;
//function Cx(lPos: integer): integer;
//var
//f: single;
//begin
//f := (lPos-lMin)*lSlope;
//if f <= 0 then
//result := 0
//else if f >= 255 then
//result := 255
//else
//result := round(f);
//end; //nested proc Cx
//begin
//lMin := LUTCenter-abs(LUTWidth/2);
//lMax := LUTCenter+abs(LUTWidth/2);
//if lMax <= lMin then
//lSlope := 1
//else
//lSlope := 255/(lMax-lMin);
//for lInc := 0 to 255 do
//lLUT[lInc] := LUTrgba(lIndex,Cx(lInc));
//end;
//
//void CreateColorTable (int lIndex, GLuint TransferTexture){// Load image data
//lLUT = new unsigned char[__imageHeight*__imageWidth*__imageDepth*4];
//
//setlength(lLUT,256*4);
//LoadLUT (lIndex,127,255, lLUT);
//glGenTextures(1, @TransferTexture);
//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//glBindTexture(GL_TEXTURE_1D, TransferTexture);
//glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, @lLUT[0]);
//lLUT := nil;
//}

void initgl (unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){
    
    glEnable(GL_CULL_FACE);
    glClearColor(ClearColor[1],ClearColor[2],ClearColor[3], 0);
    
    Load3DTextures(intensityTexture3D);
//    CreateColorTable(LUTIndex, TransferTexture);
    // Load the vertex and fragment raycasting programs
    glslprogram = initShaderWithFile("rc.vert", "rc.frag");
    // Create the to FBO's one for the backside of the volumecube and one for the finalimage rendering
    glGenFramebuffersEXT(1, &frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,frameBuffer);
    glGenTextures(1, &backFaceBuffer);
    glBindTexture(GL_TEXTURE_2D, backFaceBuffer);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backFaceBuffer, 0);
    glGenTextures(1, &finalImage);
    glBindTexture(GL_TEXTURE_2D, finalImage);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glGenRenderbuffersEXT(1, &renderBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}

void drawUnitQuad(void){
    //stretches image in view space.
    
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(0,0);
    glTexCoord2f(1,0);
    glVertex2f(1,0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(0, 1);
    glEnable(GL_DEPTH_TEST);
}

// display the final image on the screen
void renderBufferToScreen(void){
    glClear( GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,finalImage);
    //use next line instead of previous to illustrate one-pass rendering
    //glBindTexture(GL_TEXTURE_2D,backFaceBuffer);
    reshapeOrtho(WINDOW_WIDTH, WINDOW_HEIGHT);
    drawUnitQuad();
    glDisable(GL_TEXTURE_2D);
}

// render the backface to the offscreen buffer backFaceBuffer
void renderBackFace(void){
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backFaceBuffer, 0);
    glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glMatrixMode(GL_MODELVIEW);
    glScalef(ProportionalScale[1],ProportionalScale[2],ProportionalScale[3]);
    drawQuads(1.0,1.0,1.0);
    glDisable(GL_CULL_FACE);
}

void uniform1f( std::string name, int value){
    glUniform1f(glGetUniformLocation(glslprogram, (const char*)name.c_str()), value);
}

void uniform1i( std::string name, int value){
    glUniform1i(glGetUniformLocation(glslprogram, (const char*)name.c_str()), value);
}

void uniform3fv( std::string name, int v1, int v2, int v3){
    glUniform3f(glGetUniformLocation(glslprogram, (const char*)name.c_str()), v1, v2, v3);
}

void rayCasting(void){
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, finalImage, 0);
    glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT );
    // backFaceBuffer -> texture0
    glActiveTexture( GL_TEXTURE0 );
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backFaceBuffer);
    //gradientTexture -> texture1
    glActiveTexture( GL_TEXTURE1 );
    //glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D,gradientTexture3D);
    //TransferTexture -> texture2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, TransferTexture);
    //intensityTexture -> texture3
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture(GL_TEXTURE_3D,intensityTexture3D);
    glUseProgram(glslprogram);
    uniform1f( "stepSize", stepSize );
    uniform1f( "viewWidth", WINDOW_WIDTH );
    uniform1f( "viewHeight", WINDOW_HEIGHT );
    uniform1i( "backFace", 0 );		// backFaceBuffer -> texture0
    uniform1i( "gradientVol", 1 );	// gradientTexture -> texture2
    uniform1i( "intensityVol", 3 );
    uniform1i( "showGradient",showGradient);
    uniform1i( "TransferTexture",2);
    uniform1f( "edgeThresh", edgeThresh );
    uniform1f( "edgeExp", edgeExp );
    uniform1f( "DitherRay", DitherRay );
    uniform1f( "boundExp", boundExp );
    uniform3fv("clearColor",ClearColor[1],ClearColor[2],ClearColor[3]);
    uniform1f( "isRGBA", true);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glMatrixMode(GL_MODELVIEW);
    glScalef(1,1,1);
    drawQuads(1.0,1.0,1.0);
    glDisable(GL_CULL_FACE);
    glUseProgram(0);
    //glActiveTexture( GL_TEXTURE1 );
    //glDisable(GL_TEXTURE_3D);
    glActiveTexture( GL_TEXTURE0 );
    glDisable(GL_TEXTURE_2D);
}

//Redraw image
void DisplayGL(void){
    
    glClearColor(ClearColor[1],ClearColor[2],ClearColor[3], 0);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    enableRenderbuffers();
    glTranslatef(0,0,-Distance);
    glRotatef(90-Elevation,-1,0,0);
    glRotatef(Azimuth,0,0,1);
    glTranslatef(-ProportionalScale[1]/2,-ProportionalScale[2]/2,-ProportionalScale[3]/2);
    renderBackFace();
    rayCasting();
    disableRenderBuffers();
    renderBufferToScreen();
    //next, you will need to execute SwapBuffers
}



void displayUsability(void){
    std::cout << "Welcome to the volume render. To navigate the volume:" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Movement"<< std::endl;
    std::cout << "Up:\t\t\t\t\tW" << std::endl;
    std::cout << "Down:\t\t\t\t\tS" << std::endl;
    std::cout << "Left:\t\t\t\t\tA" << std::endl;
    std::cout << "Right:\t\t\t\t\tD" << std::endl;
    std::cout << std::endl;
    std::cout << "Zoom"<< std::endl;
    std::cout << "In:\t\t\t\t\tR" << std::endl;
    std::cout << "Out:\t\t\t\t\tF" << std::endl;
    std::cout << std::endl;
    std::cout << "Rotation"<< std::endl;
    std::cout << "Up:\t\t\t\t\tI" << std::endl;
    std::cout << "Down:\t\t\t\t\tK" << std::endl;
    std::cout << "Left:\t\t\t\t\tJ" << std::endl;
    std::cout << "Right:\t\t\t\t\tL" << std::endl;
    std::cout << std::endl;
    std::cout << "Quit"<< std::endl;
    std::cout << "\t\t\t\t\tQ" << std::endl;
    std::cout << "\t\t\t\t\tEsc" << std::endl;


}

void MouseClick(int button, int state, int x, int y){
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
        case 119 : cameraY-=1; break;
            //down
        case 115 : cameraY +=1; break;
            //left
        case 97 :  cameraX +=1; break;
            //right
        case 100 : cameraX -=1; break;
            //zoom in
        case 114 : cameraZ +=1; break;
            //zoom out
        case 102 : cameraZ -=1; break;
            
        case 106 : cameraYDegrees +=1; break;
            
        case 108 : cameraYDegrees -=1; break;
            
        case 105 : cameraXDegrees +=1; break;
            
        case 107 : cameraXDegrees -=1; break;
            
            
        default: std::cout << "unknown key pressed : " << (int)character << std::endl;
	}
}

void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth)
{
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition (0, 0);
	glutCreateWindow("Data Visualiser");
    
    initgl(dataSet, imageWidth, imageHeight, imageDepth);
    
    glutMouseFunc(&MouseClick);
    glutKeyboardFunc(input);
	glutDisplayFunc(DisplayGL);
	//glutIdleFunc(Update);
	glutMainLoop();
}

#endif

#ifdef MARCHING_CUBES



#endif

#ifdef PLANE_PLOT
// this is the safe ugly option currently

GLuint * textures;

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

void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){    
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

void DisplayUsability(void){
    std::cout << "Welcome to the volume render. To navigate the volume:" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Movement"<< std::endl;
    std::cout << "Up:\t\t\t\t\tW" << std::endl;
    std::cout << "Down:\t\t\t\t\tS" << std::endl;
    std::cout << "Left:\t\t\t\t\tA" << std::endl;
    std::cout << "Right:\t\t\t\t\tD" << std::endl;
    std::cout << std::endl;
    std::cout << "Zoom"<< std::endl;
    std::cout << "In:\t\t\t\t\tR" << std::endl;
    std::cout << "Out:\t\t\t\t\tF" << std::endl;
    std::cout << std::endl;
    std::cout << "Rotation"<< std::endl;
    std::cout << "Up:\t\t\t\t\tI" << std::endl;
    std::cout << "Down:\t\t\t\t\tK" << std::endl;
    std::cout << "Left:\t\t\t\t\tJ" << std::endl;
    std::cout << "Right:\t\t\t\t\tL" << std::endl;
    std::cout << std::endl;
    std::cout << "Quit"<< std::endl;
    std::cout << "\t\t\t\t\tQ" << std::endl;
    std::cout << "\t\t\t\t\tEsc" << std::endl;
    
    
}

void Init(void)
{    
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_DEPTH_TEST);
    
    glClearColor(clearcolour[0], clearcolour[1], clearcolour[2], clearcolour[3]);
    
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //glClearColor(1.0, 1.0, 1.0, 1.0);
	//glColor4d(0.0, 0.0, 0.0, 1.0);
    //glPointSize(3.0);
    
    glShadeModel(GL_SMOOTH);
    
    //    glEnable (GL_STENCIL_TEST);
    //    glStencilFunc (GL_ALWAYS, 0x1, 0x1);
    //    glStencilFunc (GL_EQUAL, 0x0, 0x1);
    //    
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(90, 3.0/3.0, 0.1, 100.0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT,GL_FASTEST);
    
    
    glDepthFunc(GL_LEQUAL);
    
    glDisable(GL_DEPTH_TEST);
    
    glDisable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    glMatrixMode(GL_MODELVIEW);
    
    DisplayUsability();
    
	glLoadIdentity();
}

//void glutPrint(float x, float y, float z, void * font, char* text, float r, float g, float b, float a) 
//{ 
//    if(!text || !strlen(text)) return; 
//    bool blending = false; 
//    if(glIsEnabled(GL_BLEND)) blending = true; 
//    glEnable(GL_BLEND); 
//    glColor3f(r,g,b); 
//	glRasterPos3d(x, y, z);
//    while (*text) { 
//        glutBitmapCharacter(font, *text); 
//        text++; 
//    } 
//    if(!blending) glDisable(GL_BLEND);	
//}  

void Display(void)
{	
    glShadeModel(GL_FLAT);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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
        
        glColor4f(colour[0], colour[1], colour[2], colour[3]);
        
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

void MouseClick(int button, int state, int x, int y){
	if (button==GLUT_LEFT && state==GLUT_DOWN) {
        //on mouse click toggle paused
        std::cout << "mouse clicked at " << x << " and " << y << std::endl;
	}
}

void Input(unsigned char character, int xx, int yy) {
	switch(character) {
            //case ' ' : changePaused(); break; 
		case 27  : exit(0); break;
            // q or esc to quit    
        case 113 : exit(0); break;
            //up
        case 119 : cameraY-=1; break;
            //down
        case 115 : cameraY +=1; break;
            //left
        case 97 :  cameraX +=1; break;
            //right
        case 100 : cameraX -=1; break;
            //zoom in
        case 114 : cameraZ +=1; break;
            //zoom out
        case 102 : cameraZ -=1; break;
            
        case 106 : cameraYDegrees +=1; break;
            
        case 108 : cameraYDegrees -=1; break;
            
        case 105 : cameraXDegrees +=1; break;
            
        case 107 : cameraXDegrees -=1; break;
            
            
        default: std::cout << "unknown key pressed : " << (int)character << std::endl;
	}
}

void IncreaseRotation(void){
	degrees += 0.25;
	if (degrees > 360) {
		degrees -= 360;
	}
}

void Wait(float seconds)
{
	clock_t endWait;
	endWait = clock_t(clock () + seconds * CLOCKS_PER_SEC);
	while (clock() < endWait) {}
}

void Update(void)
{	
    // Wait and Update the clock
	clock_t clocks_elapsed = clock() - clock_ticks; 
	if ((float) clocks_elapsed < (float) CLOCKS_PER_SEC / (float) FRAMES_PER_SEC)
		Wait(((float)CLOCKS_PER_SEC / (float) FRAMES_PER_SEC - (float) clocks_elapsed)/(float) CLOCKS_PER_SEC);
	//clock_ticks = clock();
	//IncreaseRotation();
    
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

    StoreDataSet(dataSet, imageWidth, imageHeight, imageDepth);
    
    glutMouseFunc(&MouseClick);
    glutKeyboardFunc(Input);
	glutDisplayFunc(Display);
	glutIdleFunc(Update);
	glutMainLoop();
}

#endif