/*
 *  openGLVisualiser.cpp
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


#include "openGLVisualiser.h"

const int FRAMES_PER_SEC = 10;
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

#ifdef MARCHING_CUBES_GOOD

#include "vec3f.h"
#include "marchingcubes.h"

double *** voxels;
vector<vertex> vertices;

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

void MouseClick(int button, int state, int x, int y){
	if (button==GLUT_LEFT && state==GLUT_DOWN) {
        //on mouse click toggle paused
        std::cout << "mouse clicked at " << x << " and " << y << std::endl;
        cameraYDegrees = y;
        cameraXDegrees = x;
	}
}

void Input(unsigned char character, int xx, int yy) {
	switch(character) {
            //case ' ' : changePaused(); break; 
		case 27  : exit(0); break;
            // q or esc to quit    
        case 113 : exit(0); break;
            //up
        case 115 : cameraY+=5; break;
            //down
        case 119 : cameraY -=5; break;
            //left
        case 100 :  cameraX -=5; break;
            //right
        case 97 : cameraX +=5; break;
            //zoom in
        case 114 : cameraZ +=5; break;
            //zoom out
        case 102 : cameraZ -=5; break;
            
        case 106 : cameraYDegrees +=1; break;
            
        case 108 : cameraYDegrees -=1; break;
            
        case 105 : cameraXDegrees +=1; break;
            
        case 107 : cameraXDegrees -=1; break;
            
            
        default: std::cout << "unknown key pressed : " << (int)character << std::endl;
	}
}

void Display(void)
{	
    //move camera view
    glMatrixMode(GL_PROJECTION);
    
    glTranslated(cameraX, cameraY, cameraZ);
    cameraX = 0;
    cameraY = 0;
    cameraZ = 0;
    
    glMatrixMode(GL_MODELVIEW);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Create light components
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
    
    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    
    //Set material properties which will be assigned by glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
    
    //if you want to see lighting call this earlier
    glLoadIdentity();
    
    glTranslated(-__imageHeight/2, -__imageWidth/2, -__imageDepth*8);
    //flip the embryo upside down and turn about face
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    
    //rotate the object into position
    glRotatef(cameraXDegrees,1.0f,0.0f,0.0f);		// Rotate On The X Axis
    glRotatef(cameraYDegrees,0.0f,1.0f,0.0f);		// Rotate On The Y Axis
    
    //stretch it depthwise so our foetus has some real volume
    //glScalef(0.5, 0.5, 2.5);
    //glColor4f(0.1, 0.5, 0.0f, 0.8);
    
    //draw foetus
    glPushMatrix(); 
    vector<vertex>::iterator it;
        glBegin(GL_TRIANGLES);
        for(it = vertices.begin(); it < vertices.end(); it++) {
            glNormal3d(it->normal_x, it->normal_y, it->normal_z);
                //scaling factor for z
            glVertex3d(it->x, it->y, it->z);
        }
        glEnd();
    glPopMatrix(); 
    
    
    glutSwapBuffers(); 
}


void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){    
    __imageWidth = imageWidth;
    __imageHeight = imageHeight;
    __imageDepth = imageDepth;
    
    voxels = new double**[__imageWidth];
    for (int x = 0; x < __imageWidth; x++) {
        voxels[x] = new double*[__imageHeight];
        for (int y = 0; y < __imageHeight; y++) {
            voxels[x][y] = new double[__imageDepth];
            for (int z = 0; z < __imageDepth; z++) {
                voxels[x][y][z] = dataSet[4*((z*__imageWidth*__imageHeight) + (y*__imageWidth) + x)];
            }
        }
    }
    return;
}

void Init(void){
    //glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
    
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(90, 1, 0.1, 800.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);    
    //glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glMatrixMode(GL_MODELVIEW);
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
    
    vertices = runMarchingCubes(voxels, __imageWidth, __imageWidth, __imageDepth, 1, 1, 1, 32.0);
    
    glutMouseFunc(&MouseClick);
    glutKeyboardFunc(Input);
	glutDisplayFunc(Display);
	glutIdleFunc(Update);
    DisplayUsability();
	glutMainLoop();
}

#endif

#ifdef MARCHING_CUBES_BAD

struct GLvector
{
    GLfloat fX;
    GLfloat fY;
    GLfloat fZ;     
};

//These tables are used so that everything can be done in little loops that you can look at all at once
// rather than in pages and pages of unrolled code.

//a2fVertexOffset lists the positions, relative to vertex0, of each of the 8 vertices of a cube
static const GLfloat a2fVertexOffset[8][3] =
{
    {0.0, 0.0, 0.0},{1.0, 0.0, 0.0},{1.0, 1.0, 0.0},{0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},{1.0, 0.0, 1.0},{1.0, 1.0, 1.0},{0.0, 1.0, 1.0}
};

//a2iEdgeConnection lists the index of the endpoint vertices for each of the 12 edges of the cube
static const GLint a2iEdgeConnection[12][2] = 
{
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

//a2fEdgeDirection lists the direction vector (vertex1-vertex0) for each edge in the cube
static const GLfloat a2fEdgeDirection[12][3] =
{
    {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0},
    {1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{-1.0, 0.0, 0.0},{0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},{0.0, 0.0, 1.0},{ 0.0, 0.0, 1.0},{0.0,  0.0, 1.0}
};

//a2iTetrahedronEdgeConnection lists the index of the endpoint vertices for each of the 6 edges of the tetrahedron
static const GLint a2iTetrahedronEdgeConnection[6][2] =
{
    {0,1},  {1,2},  {2,0},  {0,3},  {1,3},  {2,3}
};

//a2iTetrahedronEdgeConnection lists the index of verticies from a cube 
// that made up each of the six tetrahedrons within the cube
static const GLint a2iTetrahedronsInACube[6][4] =
{
    {0,5,1,6},
    {0,1,2,6},
    {0,2,3,6},
    {0,3,7,6},
    {0,7,4,6},
    {0,4,5,6},
};

static const GLfloat afAmbientWhite [] = {0.25, 0.25, 0.25, 1.00}; 
static const GLfloat afAmbientRed   [] = {0.25, 0.00, 0.00, 1.00}; 
static const GLfloat afAmbientGreen [] = {0.00, 0.25, 0.00, 1.00}; 
static const GLfloat afAmbientBlue  [] = {0.00, 0.00, 0.25, 1.00}; 
static const GLfloat afDiffuseWhite [] = {0.75, 0.75, 0.75, 1.00}; 
static const GLfloat afDiffuseRed   [] = {0.75, 0.00, 0.00, 1.00}; 
static const GLfloat afDiffuseGreen [] = {0.00, 0.75, 0.00, 1.00}; 
static const GLfloat afDiffuseBlue  [] = {0.00, 0.00, 0.75, 1.00}; 
static const GLfloat afSpecularWhite[] = {1.00, 1.00, 1.00, 1.00}; 
static const GLfloat afSpecularRed  [] = {1.00, 0.25, 0.25, 1.00}; 
static const GLfloat afSpecularGreen[] = {0.25, 1.00, 0.25, 1.00}; 
static const GLfloat afSpecularBlue [] = {0.25, 0.25, 1.00, 1.00}; 


GLenum    ePolygonMode = GL_FILL;
GLint     iDataSetSize = 16;
GLfloat   fStepSize = 1.0;
GLfloat   fTargetValue = 5.0;//0.5;//48.0;
GLfloat   fTime = 0.0;
GLvector  sSourcePoint[3];
GLboolean bSpin = true;
GLboolean bMove = true;
GLboolean bLight = true;

GLfloat fSample1(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fSample2(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fSample3(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fSample4(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat (*fSample)(GLfloat fX, GLfloat fY, GLfloat fZ) = fSample1;

GLvoid vMarchingCubes();
GLvoid vMarchCube1(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);
GLvoid vMarchCube2(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);
GLvoid (*vMarchCube)(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale) = vMarchCube1;

void StoreDataSet(unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth){    
    __imageWidth = imageWidth;
    __imageHeight = imageHeight;
    __imageDepth = imageDepth;
        
    __dataSet = new unsigned char[__imageWidth*__imageHeight*4*__imageDepth];
    
    for (int i = 0; i < __imageWidth*__imageHeight*4*imageDepth; i++) {
        __dataSet[i]=dataSet[i];
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


void Display(void)
{	
    glShadeModel(GL_FLAT);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
    //glutPrint(0, 0, 1, GLUT_BITMAP_TIMES_ROMAN_24, "bananas", 1, 0.5, 0, 0.1);
    glTranslated(cameraX, cameraY, cameraZ);
    glTranslated(0.0f, 0.0f, 0.0f);
    //glScalef(0.01, 0.01, 0.01);
    glRotatef(cameraXDegrees,1.0f,0.0f,0.0f);		// Rotate On The X Axis
    glRotatef(cameraYDegrees,0.0f,1.0f,0.0f);		// Rotate On The Y Axis
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    
    glPushMatrix(); 
    
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glutWireCube(1.0); 
    glPopAttrib(); 
    
    
    glPushMatrix(); 
    glTranslatef(-0.5, -0.5, -0.5);
    glScalef(0.01, 0.01, 0.01);
    glBegin(GL_TRIANGLES);
        vMarchingCubes();
    glEnd();
    glPopMatrix(); 
    
    
    glPopMatrix(); 
    
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

void Init(void){
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(90, 1, 0.1, 100.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);    
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glMatrixMode(GL_MODELVIEW);
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
    
    //move data plot?
    
	glutPostRedisplay();
}


void plotMain(int argc, char ** argv, unsigned char * dataSet, int imageWidth, int imageHeight, int imageDepth)
{
    GLfloat afPropertiesAmbient [] = {0.50, 0.50, 0.50, 1.00}; 
    GLfloat afPropertiesDiffuse [] = {0.75, 0.75, 0.75, 1.00}; 
    GLfloat afPropertiesSpecular[] = {1.00, 1.00, 1.00, 1.00}; 
    
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutInitWindowPosition (0, 0);
	glutCreateWindow("Marching Cubes");
    
    Init();
    
    fSample = fSample4;

    StoreDataSet(dataSet, imageWidth, imageHeight, imageDepth);
    
    glutMouseFunc(&MouseClick);
    glutKeyboardFunc(Input);
	glutDisplayFunc(Display);
	glutIdleFunc(Update);
    
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, ePolygonMode);
    
    glLightfv( GL_LIGHT0, GL_AMBIENT,  afPropertiesAmbient); 
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  afPropertiesDiffuse); 
    glLightfv( GL_LIGHT0, GL_SPECULAR, afPropertiesSpecular); 
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0); 
    
    glEnable( GL_LIGHT0 ); 
    
    glMaterialfv(GL_BACK,  GL_AMBIENT,   afAmbientGreen); 
    glMaterialfv(GL_BACK,  GL_DIFFUSE,   afDiffuseGreen); 
    glMaterialfv(GL_FRONT, GL_AMBIENT,   afAmbientBlue); 
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   afDiffuseBlue); 
    glMaterialfv(GL_FRONT, GL_SPECULAR,  afSpecularWhite); 
    glMaterialf( GL_FRONT, GL_SHININESS, 25.0); 

    
    DisplayUsability();
	glutMainLoop();
}



//fGetOffset finds the approximate point of intersection of the surface
// between two points with the values fValue1 and fValue2
GLfloat fGetOffset(GLfloat fValue1, GLfloat fValue2, GLfloat fValueDesired)
{
    GLdouble fDelta = fValue2 - fValue1;
    
    if(fDelta == 0.0)
    {
        return 0.5;
    }
    return (fValueDesired - fValue1)/fDelta;
}


//vGetColor generates a color from a given position and normal of a point
GLvoid vGetColor(GLvector &rfColor, GLvector &rfPosition, GLvector &rfNormal)
{
    GLfloat fX = rfNormal.fX;
    GLfloat fY = rfNormal.fY;
    GLfloat fZ = rfNormal.fZ;
    rfColor.fX = (fX > 0.0 ? fX : 0.0) + (fY < 0.0 ? -0.5*fY : 0.0) + (fZ < 0.0 ? -0.5*fZ : 0.0);
    rfColor.fY = (fY > 0.0 ? fY : 0.0) + (fZ < 0.0 ? -0.5*fZ : 0.0) + (fX < 0.0 ? -0.5*fX : 0.0);
    rfColor.fZ = (fZ > 0.0 ? fZ : 0.0) + (fX < 0.0 ? -0.5*fX : 0.0) + (fY < 0.0 ? -0.5*fY : 0.0);
}

GLvoid vNormalizeVector(GLvector &rfVectorResult, GLvector &rfVectorSource)
{
    GLfloat fOldLength;
    GLfloat fScale;
    
    fOldLength = sqrtf( (rfVectorSource.fX * rfVectorSource.fX) +
                       (rfVectorSource.fY * rfVectorSource.fY) +
                       (rfVectorSource.fZ * rfVectorSource.fZ) );
    
    if(fOldLength == 0.0)
    {
        rfVectorResult.fX = rfVectorSource.fX;
        rfVectorResult.fY = rfVectorSource.fY;
        rfVectorResult.fZ = rfVectorSource.fZ;
    }
    else
    {
        fScale = 1.0/fOldLength;
        rfVectorResult.fX = rfVectorSource.fX*fScale;
        rfVectorResult.fY = rfVectorSource.fY*fScale;
        rfVectorResult.fZ = rfVectorSource.fZ*fScale;
    }
}


//fSample1 finds the distance of (fX, fY, fZ) from three moving points
GLfloat fSample1(GLfloat fX, GLfloat fY, GLfloat fZ)
{
    GLdouble fResult = 0.0;
    GLdouble fDx, fDy, fDz;
    fDx = fX - sSourcePoint[0].fX;
    fDy = fY - sSourcePoint[0].fY;
    fDz = fZ - sSourcePoint[0].fZ;
    fResult += 0.5/(fDx*fDx + fDy*fDy + fDz*fDz);
    
    fDx = fX - sSourcePoint[1].fX;
    fDy = fY - sSourcePoint[1].fY;
    fDz = fZ - sSourcePoint[1].fZ;
    fResult += 1.0/(fDx*fDx + fDy*fDy + fDz*fDz);
    
    fDx = fX - sSourcePoint[2].fX;
    fDy = fY - sSourcePoint[2].fY;
    fDz = fZ - sSourcePoint[2].fZ;
    fResult += 1.5/(fDx*fDx + fDy*fDy + fDz*fDz);
    
    return fResult;
}

//fSample2 finds the distance of (fX, fY, fZ) from three moving lines
GLfloat fSample2(GLfloat fX, GLfloat fY, GLfloat fZ)
{
    GLdouble fResult = 0.0;
    GLdouble fDx, fDy, fDz;
    fDx = fX - sSourcePoint[0].fX;
    fDy = fY - sSourcePoint[0].fY;
    fResult += 0.5/(fDx*fDx + fDy*fDy);
    
    fDx = fX - sSourcePoint[1].fX;
    fDz = fZ - sSourcePoint[1].fZ;
    fResult += 0.75/(fDx*fDx + fDz*fDz);
    
    fDy = fY - sSourcePoint[2].fY;
    fDz = fZ - sSourcePoint[2].fZ;
    fResult += 1.0/(fDy*fDy + fDz*fDz);
    
    return fResult;
}


//fSample2 defines a height field by plugging the distance from the center into the sin and cos functions
GLfloat fSample3(GLfloat fX, GLfloat fY, GLfloat fZ)
{
    GLfloat fHeight = 20.0*(fTime + sqrt((0.5-fX)*(0.5-fX) + (0.5-fY)*(0.5-fY)));
    fHeight = 1.5 + 0.1*(sinf(fHeight) + cosf(fHeight));
    GLdouble fResult = (fHeight - fZ)*50.0;
    
    return fResult;
}

GLfloat fSample4(GLfloat fX, GLfloat fY, GLfloat fZ)
{    
    return __dataSet[4*(((int)fZ*__imageHeight*__imageWidth) + ((int)fY*__imageWidth) + (int)fX)];
    //return (6.0*fX - 3.0)*sin(6.0*fY - 3.0) + sin(pow(0.5*fZ - 2.0, 3.0));
}


//vGetNormal() finds the gradient of the scalar field at a point
//This gradient can be used as a very accurate vertx normal for lighting calculations
GLvoid vGetNormal(GLvector &rfNormal, GLfloat fX, GLfloat fY, GLfloat fZ)
{
    rfNormal.fX = fSample(fX-1, fY, fZ) - fSample(fX+1, fY, fZ);
    rfNormal.fY = fSample(fX, fY-1, fZ) - fSample(fX, fY+1, fZ);
    rfNormal.fZ = fSample(fX, fY, fZ-1) - fSample(fX, fY, fZ+1);
    vNormalizeVector(rfNormal, rfNormal);
}


//vMarchCube1 performs the Marching Cubes algorithm on a single cube
GLvoid vMarchCube1(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale)
{
    extern GLint aiCubeEdgeFlags[256];
    extern GLint a2iTriangleConnectionTable[256][16];
    
    GLint iCorner, iVertex, iVertexTest, iEdge, iTriangle, iFlagIndex, iEdgeFlags;
    GLfloat fOffset;
    GLvector sColor;
    GLfloat afCubeValue[8];
    GLvector asEdgeVertex[12];
    GLvector asEdgeNorm[12];
    
    //Make a local copy of the values at the cube's corners
    for(iVertex = 0; iVertex < 8; iVertex++)
    {
        afCubeValue[iVertex] = fSample(fX + a2fVertexOffset[iVertex][0]*fScale,
                                       fY + a2fVertexOffset[iVertex][1]*fScale,
                                       fZ + a2fVertexOffset[iVertex][2]*fScale);
    }
    
    //Find which vertices are inside of the surface and which are outside
    iFlagIndex = 0;
    for(iVertexTest = 0; iVertexTest < 8; iVertexTest++)
    {
        if(afCubeValue[iVertexTest] > 250.0) 
            iFlagIndex |= 1<<iVertexTest;
    }
    
    //Find which edges are intersected by the surface
    iEdgeFlags = aiCubeEdgeFlags[iFlagIndex];
    
    //If the cube is entirely inside or outside of the surface, then there will be no intersections
    if(iEdgeFlags == 0) 
    {
        return;
    }
    
    //Find the point of intersection of the surface with each edge
    //Then find the normal to the surface at those points
    for(iEdge = 0; iEdge < 12; iEdge++)
    {
        //if there is an intersection on this edge
        if(iEdgeFlags & (1<<iEdge))
        {
            fOffset = fGetOffset(afCubeValue[ a2iEdgeConnection[iEdge][0] ], 
                                 afCubeValue[ a2iEdgeConnection[iEdge][1] ], fTargetValue);
            
            asEdgeVertex[iEdge].fX = fX + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][0]  +  fOffset * a2fEdgeDirection[iEdge][0]) * fScale;
            asEdgeVertex[iEdge].fY = fY + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][1]  +  fOffset * a2fEdgeDirection[iEdge][1]) * fScale;
            asEdgeVertex[iEdge].fZ = fZ + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0] ][2]  +  fOffset * a2fEdgeDirection[iEdge][2]) * fScale;
            
            vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].fX, asEdgeVertex[iEdge].fY, asEdgeVertex[iEdge].fZ);
        }
    }
    
    
    //Draw the triangles that were found.  There can be up to five per cube
    for(iTriangle = 0; iTriangle < 5; iTriangle++)
    {
        if(a2iTriangleConnectionTable[iFlagIndex][3*iTriangle] < 0)
            break;
        
        for(iCorner = 0; iCorner < 3; iCorner++)
        {
            iVertex = a2iTriangleConnectionTable[iFlagIndex][3*iTriangle+iCorner];
            
            vGetColor(sColor, asEdgeVertex[iVertex], asEdgeNorm[iVertex]);
            glColor3f(sColor.fX, sColor.fY, sColor.fZ);
            glNormal3f(asEdgeNorm[iVertex].fX,   asEdgeNorm[iVertex].fY,   asEdgeNorm[iVertex].fZ);
            glVertex3f(asEdgeVertex[iVertex].fX, asEdgeVertex[iVertex].fY, asEdgeVertex[iVertex].fZ);
        }
    }
}


//vMarchingCubes iterates over the entire dataset, calling vMarchCube on each cube
GLvoid vMarchingCubes()
{
    GLint iX, iY, iZ;
    for(iX = 0; iX < __imageWidth; iX++)
        for(iY = 0; iY < __imageHeight; iY++)
            for(iZ = 0; iZ < __imageDepth; iZ++)
            {
                vMarchCube(iX, iY, iZ, 1);
            }
}

// For any edge, if one vertex is inside of the surface and the other is outside of the surface
//  then the edge intersects the surface
// For each of the 8 vertices of the cube can be two possible states : either inside or outside of the surface
// For any cube the are 2^8=256 possible sets of vertex states
// This table lists the edges intersected by the surface for all 256 possible vertex states
// There are 12 edges.  For each entry in the table, if edge #n is intersected, then bit #n is set to 1

GLint aiCubeEdgeFlags[256]=
{
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00, 
    0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90, 
    0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30, 
    0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0, 
    0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60, 
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0, 
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0, 
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460, 
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0, 
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230, 
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190, 
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

//  For each of the possible vertex states listed in aiCubeEdgeFlags there is a specific triangulation
//  of the edge intersection points.  a2iTriangleConnectionTable lists all of them in the form of
//  0-5 edge triples with the list terminated by the invalid value -1.
//  For example: a2iTriangleConnectionTable[3] list the 2 triangles formed when corner[0] 
//  and corner[1] are inside of the surface, but the rest of the cube is not.
//
//  I found this table in an example program someone wrote long ago.  It was probably generated by hand

GLint a2iTriangleConnectionTable[256][16] =  
{
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};


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