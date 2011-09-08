/*
 *  RGBAUtilities.h
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 13/07/11.
 *  Copyright (C) 2011 by Beau Johnston.
 *
 *  Please email me if you have any comments, suggestions or advice:
 *                              beau@inbeta.org
 *
 *  read_png_file & write_png_file functionality, Guillaume Cottenceau, 2002-2010
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


#ifndef RGBA_UTILS
#define RGBA_UTILS

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#define PNG_DEBUG 3

//define our data types, the following numbers denote the number of bits
//e.g. int8 uses a signed 8 bit
#define int8 signed char
#define int16 signed short
#define int32 signed int
#define int64 signed long
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long

//supported for apple OSX and other variations of UNIX
#if defined (__APPLE__)  && defined (__MACH__)
    #include <libpng/png.h>
#else
    #include <png.h>
#endif

class RGBAUtilities {
    
private:
    //private variables
    
    int x, y;
    png_structp pngPtr;
    png_infop infoPtr;
    int numberOfPasses;
    png_bytep * rowPointers;
    uint32 imageLength, imageWidth, config, bitsPerSample, samplesPerPixel, bitsPerPixel, imageBitSize, imageSize;
    uint64 linebytes;
    
    union FloatAndByte
    {
        float   f;
        uint8   c[0];
    };
    
    //private functions
    void abort_(const char * s, ...);
    void initializeRedTileRowPtr(void);
    void printImageSpecs(void);
    void convolutedSetImage(uint8* image);
    uint8* convolutedGetImage(void);


public:
    RGBAUtilities();
    ~RGBAUtilities();
    void readPngFile(char* file_name);
    void writePngFile(char* file_name);
    //normalizing/denormalizing and testing normalization only works with the getImage/setImage functions and even then cannot currently be used due to rounding
    float* normalizeImage(uint8*);
    uint8* denormalizeImage(float*);
    bool allPixelsAreNormal(uint8*);
    
    uint8* getImage(void);
    void setImage(uint8*);
    void setImageFromFloat(uint8* image);
    void clearImageBuffer();
    
    float* norm(float* input, uint32 imageSize);
    float* denorm(float* input, uint32 imageSize);
    float* upcastToFloat(uint8* input, uint32 imageSize);
    uint8* downCastToByte(float* input, uint32 imageSize);
    float* upcastToFloatAndNormalize(uint8* input, uint32 imageSize);
    uint8* downcastToByteAndDenormalize(float* input, uint32 imageSize);
    float* multiplexToFloat(uint8* data, int imageSize);
    uint8* demultToBytes(float* data, int imageSize);
    
    uint32 getImageSizeInFloats(void);
    uint32 getImageLength(void);
    uint32 getImageHeight(void);
    uint32 getImageWidth(void);
    uint32 getConfig(void);
    uint32 getBitsPerSample(void);
    uint32 getSamplesPerPixel(void);
    uint32 getImageRowPitch(void);
    uint32 getImageSize(void);
    
    uint8* createBlackTile(void);
    void imageStatistics(uint8 * input, uint32 imageSize);
    void printImage(uint8 * input, uint32 imageSize);
    void cleanup(void);
};

#endif

