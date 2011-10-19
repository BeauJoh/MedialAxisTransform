/*
 *  myKernel.cl
 *  MedialAxisTransform
 *
 *
 *  Created by Beau Johnston on 25/08/11
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



#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef struct FCOMPLEX {float r,i;} fcomplex;


// function prototypes to avoid openCL compiler warning

fcomplex Cmul(fcomplex,fcomplex);
fcomplex Complex(float, float);
fcomplex Conjg(fcomplex);
fcomplex Cdiv(fcomplex, fcomplex);
fcomplex Cadd(fcomplex, fcomplex);
fcomplex Csub(fcomplex, fcomplex);
float4 getFillColour();
//static inline int FFT(int,long, float*,float*);
//int DFT(int,int,float*,float*);
int ClosestPower2(int);

//implementation of defined functions
fcomplex Cmul(fcomplex a, fcomplex b)
{
	fcomplex c;
	c.r=a.r*b.r-a.i*b.i;
	c.i=a.i*b.r+a.r*b.i;
	return c;
}

fcomplex Complex(float re, float im)
{
	fcomplex c;
	c.r=re;
	c.i=im;
	return c;
}

fcomplex Conjg(fcomplex z)
{
	fcomplex c;
	c.r=z.r;
	c.i = -z.i;
	return c;
}

fcomplex Cdiv(fcomplex a, fcomplex b)
{
	fcomplex c;
	float r,den;
	if (fabs(b.r) >= fabs(b.i)) {
		r=b.i/b.r;
		den=b.r+r*b.i;
		c.r=(a.r+r*a.i)/den;
		c.i=(a.i-r*a.r)/den;
	} else {
		r=b.r/b.i;
		den=b.i+r*b.r;
		c.r=(a.r*r+a.i)/den;
		c.i=(a.i*r-a.r)/den;
	}
	return c;
}

fcomplex Cadd(fcomplex a, fcomplex b)
{
	fcomplex c;
	c.r=a.r+b.r;
	c.i=a.i+b.i;
	return c;
}

fcomplex Csub(fcomplex a, fcomplex b)
{
	fcomplex c;
	c.r=a.r-b.r;
	c.i=a.i-b.i;
	return c;
}

float4 getFillColour(){
    return (float4) (1.0f,0.5f,0.0f,1);
}

/* ----------------------------------> Real Utility functions start here <------------------------------ */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL 
#define BOOL unsigned int
#endif

#ifndef NULL 
#define NULL ((void *)0)
#endif

#define FFT_FORWARD 1
#define FFT_REVERSE -1

/*
 This computes an in-place complex-to-complex FFT
 x and y are the real and imaginary arrays of 2^m points.
 dir =  1 gives forward transform
 dir = -1 gives reverse transform
 */
__local inline float8 FFT(int dir,long m,float4 xIn,float4 yIn){
    float x[4];
    float y[4];
    
    x[0] = xIn.x;
    x[1] = xIn.y;
    x[2] = xIn.z;
    x[3] = xIn.w;
    
    y[0] = yIn.x;
    y[1] = yIn.y;
    y[2] = yIn.z;
    y[3] = yIn.w;
    
	long n,i,i1,j,k,i2,l,l1,l2;
	float c1,c2,tx,ty,t1,t2,u1,u2,z;
    
	// Calculate the number of points
	n = 1;
	for (i=0;i<m;i++)
		n *= 2;
    
	// Do the bit reversal
	i2 = n >> 1;
	j = 0;
	for (i=0;i<n-1;i++) {
		if (i < j) {
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = (float)tx;
			y[j] = (float)ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
    
	// Compute the FFT
	c1 = -1.0;
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0;
		u2 = 0.0;
		for (j=0;j<l1;j++) {
			for (i=j;i<n;i+=l2) {
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = (float)(x[i] - t1);
				y[i1] = (float)(y[i] - t2);
				x[i] += (float)t1;
				y[i] += (float)t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
        c2 = (float)pow((float)((1.0 - c1) / 2.0),1/2);
		//c2 = (float)sqrt((float)((1.0f - c1) / 2.0f));
		if (dir == FFT_FORWARD){
			c2 = -c2;
        }
        c1 = (float)pow((float)((1.0+c1)/2),1/2);
		//c1 = (float)sqrt((float)((1.0 + c1) / 2.0));
	}
    
	// Scaling for forward transform
	if (dir == FFT_FORWARD) {
		for (i=0;i<n;i++) {
			x[i] /= n;
			y[i] /= n;
		}
	}
    
    float8 Out = (float8)(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
    Out.s0 = x[0];
    Out.s1 = x[1];
    Out.s2 = x[2];
    Out.s3 = x[3];
    
	Out.s4 = y[0];
    Out.s5 = y[1];
    Out.s6 = y[2];
    Out.s7 = y[3];
    
    return Out;
}

/*
 Direct fourier transform
 */
__local inline float8 DFT(int dir,int m,float4 xIn,float4 yIn)
{
    
    float x1[4];
    float y1[4];

    float x2[4];
    float y2[4];

    
    x1[0] = xIn.x;
    x1[1] = xIn.y;
    x1[2] = xIn.z;
    x1[3] = xIn.w;
    
    y1[0] = yIn.x;
    y1[1] = yIn.y;
    y1[2] = yIn.z;
    y1[3] = yIn.w;
    
    long i,k;
    float arg;
    float cosarg,sinarg;
    
    for (i=0;i<m;i++) {
        x2[i] = 0;
        y2[i] = 0;
        arg = - dir * 2.0 * 3.141592654 * (float)i / (float)m;
        for (k=0;k<m;k++) {
            cosarg = cos(k * arg);
            sinarg = sin(k * arg);
            x2[i] += (x1[k] * cosarg - y1[k] * sinarg);
            y2[i] += (x1[k] * sinarg + y1[k] * cosarg);
        }
    }
    
    /* Copy the data back */
    if (dir == 1) {
        for (i=0;i<m;i++) {
            x1[i] = x2[i] / (float)m;
            y1[i] = y2[i] / (float)m;
        }
    } else {
        for (i=0;i<m;i++) {
            x1[i] = x2[i];
            y1[i] = y2[i];
        }
    }
    
    float8 Out = (float8)(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
    Out.s0 = x1[0];
    Out.s1 = x1[1];
    Out.s2 = x1[2];
    Out.s3 = x1[3];
    
	Out.s4 = y1[0];
    Out.s5 = y1[1];
    Out.s6 = y1[2];
    Out.s7 = y1[3];
    
    return Out;
}


inline void modify(float* x, __local float* y, int size){
    for(int i = 0; i < size;i++){
        x[i] = y[i];
    }
}

//long ClosestPower2(long x){
//	long double temp=log(x)/log(2);
//	return (long)pow(2,((int)(temp+0.5)));
//}

int ClosestPower2(int x){
	//long double temp=log(x)/log(2);
	return log((float)x)/log((float)2);
}


/* --------------------------> Real work happens past this point <--------------------------------- */


__kernel
void sobel3D(__read_only image3d_t srcImg,
           __write_only image3d_t dstImg,
           sampler_t sampler,
           int width, int height, int depth)
{
    
    int x = (int)get_global_id(0);
    int y = (int)get_global_id(1);
    int z = (int)get_global_id(2);
    
    if (x >= get_image_width(srcImg) || y >= get_image_height(srcImg) || z >= get_image_depth(srcImg)){
        return;
    }
    
    /*
    float filtX[3] = {-1, 0, 1};
    float filtY[3] = {-1, 0, 1};
    float filtZ[3] = {-1, 0, 1};
    
    float dvfXYZ[3][3][3];
    float dvfYZX[3][3][3];
    float dvfZXY[3][3][3];

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            for(int k = 0; k < 3; k++){
                dvfXYZ[i][j][k] = - filtX[i] * exp(-((pow(filtX[i],2)+pow(filtY[j],2)+pow(filtZ[k],2))/2));
                dvfYZX[i][j][k] = - filtY[j] * exp(-((pow(filtX[i],2)+pow(filtY[j],2)+pow(filtZ[k],2))/2));
                dvfZXY[i][j][k] = - filtZ[k] * exp(-((pow(filtX[i],2)+pow(filtY[j],2)+pow(filtZ[k],2))/2));
            }
        }
    }
    */
    

//    float A1[6][3][3];
//    float A1i[6][3][3];
//
//    for (int sliceIWant = 0; sliceIWant < 3; sliceIWant ++){
//        for (int columnIWant = 0; columnIWant < 3; columnIWant ++){
//            
//            float * aRow = &A1[0][0][0];
//            float * aRowImag = &A1i[0][0][0];
//            
//            for (int i = 0; i < 3; i++){
//                *aRow++ = dvfXYZ[i][columnIWant][sliceIWant];
//            }
//            FFT( (short int)1, (long)3, (float*)aRow, (float*)aRowImag);
//            
//        }
//    }

//    float * aRow = 0.0f;
//    float * aRowImag = 0.0f;
//    
//    FFT( (short int)1, (long)2, aRow, aRowImag);

    //float * A1 = dvfXYZ[0][][];
    //FFT(1,3,dvfXYZ[0][0][0],dvfXYZ[0][0][0]);
    //rest goes here!
    
    //w is ignored? I believe w is included as all data types are a power of 2
    int4 startImageCoord = (int4) (get_global_id(0) - 1,
                                   get_global_id(1) - 1,
                                   get_global_id(2) - 1, 
                                   1);
    
    int4 endImageCoord   = (int4) (get_global_id(0) + 1,
                                   get_global_id(1) + 1, 
                                   //remove plus 1 to get indexing proper
                                   get_global_id(2) + 1 /* + 1*/, 
                                   1);
    
    int4 outImageCoord = (int4) (get_global_id(0),
                                 get_global_id(1),
                                 get_global_id(2), 
                                 1);
    
    if (outImageCoord.x < width && outImageCoord.y < height && outImageCoord.z < depth)
    {
        float4 thisIn = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        float4 thisOut = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        
        long thisDepth = endImageCoord.z - startImageCoord.z;
        long thisHeight = endImageCoord.y - startImageCoord.y;
        long thisWidth = endImageCoord.x - startImageCoord.x;
        //printf((char const *)"%i", (int)thisDepth);
        //int stackSize = thisDepth*thisWidth*thisHeight;

        //Create Nx * Ny * Nz array of data. i.e. embryo slices (Nx * Ny) pixels * (Nz) slices deep. Denoted by DaR real components.
//        if(thisDepth != 2 || thisHeight != 2 || thisWidth !=2)
//            printf((const char*)"wrong dimensioned image chunck!");

        float DaR[3][3][3];
        float DaI[3][3][3];
        
//        int z = startImageCoord.z;
//        for(int i = 0; i < 4; i++){
//            int y = startImageCoord.y;
//            for(int j = 0; j < 4; j++){
//                int x = startImageCoord.x;
//                for(int k = 0; k < 4; k++){
//                    if (k == 3 || j == 3 || i == 3) {
//                        DaR[i][j][k] = 0;
//                        DaI[i][j][k] = 0;
//                    }
//                    else{
//                        thisIn = read_imagef(srcImg, sampler, (int4)(x, y, z, 1));
//                        DaR[i][j][k] = thisIn.x;
//                        DaI[i][j][k] = 0.0f;
//                    }
//                    x++;
//                }
//                y++;
//            }
//            z++;
//        }
        
        //first collect the red channel
        for(int z = startImageCoord.z; z <= endImageCoord.z; z++){
            for(int y = startImageCoord.y; y <= endImageCoord.y; y++){
                for(int x = startImageCoord.x; x <= endImageCoord.x; x++){
                        thisIn = read_imagef(srcImg, sampler, (int4)(x,y,z,1));
                        DaR[z - startImageCoord.z][y - startImageCoord.y][x - startImageCoord.x] = thisIn.x;
                        DaI[z - startImageCoord.z][y - startImageCoord.y][x - startImageCoord.x] = 0.0f;
                }
            }
        }
        
        float DatR[4][4][4];
        float DatI[4][4][4];
        
        
        //set out 4 window size (need 3 for correct filter focus, however need to be dyadic for fft, solution use window 
        //size of 4*4*4 whilst only populating the 3*3*3 and padding the rest with zeros)
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                for(int k = 0; k < 4; k++){
                    if (k == 3 || j == 3 || i == 3) {
                        DatR[i][j][k] = 0;
                        DatI[i][j][k] = 0;
                    }
                    else{
                        DatR[i][j][k] = DaR[i][j][k];
                        DatI[i][j][k] = DaI[i][j][k];
                    }
                }
            }
        }
        
        //row wise fft
        for (int i = 0; i < 4; i ++) {
            for (int j = 0; j < 4; j ++) {
                float tmpRowR[4];
                float tmpRowI[4];
                
                //collect a row
                for (int k = 0; k < 4; k ++) {
                    // throw into a tmp array to do FFT upon
                    tmpRowR[k] = DatR[i][j][k];
                    tmpRowI[k] = DatI[i][j][k];
                }
                
                float4 tmpRowRFF;
                tmpRowRFF.x = tmpRowR[0];
                tmpRowRFF.y = tmpRowR[1];
                tmpRowRFF.z = tmpRowR[2];
                tmpRowRFF.w = tmpRowR[3];
                
                float4 tmpRowIFF;
                tmpRowIFF.x = tmpRowI[0];
                tmpRowIFF.y = tmpRowI[1];
                tmpRowIFF.z = tmpRowI[2];
                tmpRowIFF.w = tmpRowI[3];
                
                
                
                //apply FFT
                float8 Out = FFT(1, 2, tmpRowRFF, tmpRowIFF);
                //float8 Out = DFT(1, 4, tmpRowRFF, tmpRowIFF);

                
                tmpRowR[0] = Out.s0;
                tmpRowR[1] = Out.s1;
                tmpRowR[2] = Out.s2;
                tmpRowR[3] = Out.s3;
                
                tmpRowI[0] = Out.s4;
                tmpRowI[1] = Out.s5;
                tmpRowI[2] = Out.s6;
                tmpRowI[3] = Out.s7;
                
                // store the resulting row into original array
                for (int k = 0; k < 4; k ++) {
                    // throw into a tmp array to do FFT upon
                    DatR[i][j][k] = tmpRowR[k];
                    DatI[i][j][k] = tmpRowI[k];
                }
            }
        }
        
        
        
        
        
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                for(int k = 0; k < 4; k++){
                    if (k != 3 && j != 3 && i != 3) {
                        DaR[i][j][k] = DatR[i][j][k];
                        DaI[i][j][k] = DatI[i][j][k];
                    }
                }
            }
        }

        
        //write this channel out
        for(int z = startImageCoord.z; z <= endImageCoord.z; z++){
            for(int y = startImageCoord.y; y <= endImageCoord.y; y++){
                for(int x= startImageCoord.x; x <= endImageCoord.x; x++){
                    if(DaR[z - startImageCoord.z][y - startImageCoord.y][x - startImageCoord.x] > 0.05){                        
                        write_imagef(dstImg, outImageCoord, (float4)(DaR[z - startImageCoord.z][y - startImageCoord.y][x - startImageCoord.x],0,0,1));
                    }
                }
            }
        }
        
        
        
//        //write image chunk back
//        for(int z = startImageCoord.z; z <= endImageCoord.z; z++)
//        {
//            for(int y = startImageCoord.y; y <= endImageCoord.y; y++)
//            {
//                for(int x= startImageCoord.x; x <= endImageCoord.x; x++)
//                {
//                    if(DaR[(z-startImageCoord.z)*(thisWidth*thisHeight) + (y-startImageCoord.y)*(thisWidth) + (x-startImageCoord.x)] > 0.05){                        
//                        write_imagef(dstImg, outImageCoord,(float4)(DaR[(z-startImageCoord.z)*(thisWidth*thisHeight) + (y-startImageCoord.y)*(thisWidth) + (x-startImageCoord.x)],0.0f,0.0f,1.0f));
//                    }
//                }
//            }
//        }
        
        //FFT(1,thisWidth*thisHeight*thisDepth,datasetReal,datasetImag);
        //ThreeDimensionalFFT(1, width, height, depth, datasetReal, datasetImag);
        
    }
            
}

__kernel
void sobel(__read_only image3d_t srcImg,
           __write_only image3d_t dstImg,
           sampler_t sampler,
           int width, int height, int depth)
{
    
    //*********************************************************************
    //
    // The operator uses two 3 x 3 kernels which are convolved with the
    // original image to compute derivatives - one for horizontal changes &
    // another for vertical.
    //
    // Gx the horizontal derivative is computed using the following 3 x 3 // kernel
    //
    //      [-1     0   +1]
    // Gx = [-2     0   +2]
    //      [-1     0   +1]
    //
    // Gy the vertical derivative is computed using the following 3 x 3
    // kernel
    //
    //      [-1     -2  -1]
    // Gy=  [0      0    0]
    //      [+1     +2  +1]
    //
    // //*********************************************************************
    
    int x = (int)get_global_id(0);
    int y = (int)get_global_id(1);
    int z = (int)get_global_id(2);
    
    if (x >= get_image_width(srcImg) || y >= get_image_height(srcImg) || z >= get_image_depth(srcImg)){
        return;
    }
    
    float4 p00 = read_imagef(srcImg, sampler, (int4)(x - 1, y - 1,  z, 1)); 
    float4 p10 = read_imagef(srcImg, sampler, (int4)(x,     y - 1,  z, 1)); 
    float4 p20 = read_imagef(srcImg, sampler, (int4)(x + 1, y - 1,  z, 1));
    
    float4 p01 = read_imagef(srcImg, sampler, (int4)(x - 1, y,  z,  1));
    float4 p21 = read_imagef(srcImg, sampler, (int4)(x + 1, y,  z,  1));
    
    float4 p02 = read_imagef(srcImg, sampler, (int4)(x - 1, y + 1,  z,  1)); 
    float4 p12 = read_imagef(srcImg, sampler, (int4)(x,     y + 1,  z,  1)); 
    float4 p22 = read_imagef(srcImg, sampler, (int4)(x + 1, y + 1,  z,  1));
    
    float3 gx = -p00.xyz + p20.xyz + 2.0f * (p21.xyz - p01.xyz) -p02.xyz + p22.xyz;
    float3 gy = -p00.xyz - p20.xyz + 2.0f * (p12.xyz - p10.xyz) +p02.xyz + p22.xyz;
    
    float3  g = native_sqrt(gx * gx + gy * gy);
    
    // we could also approximate this as g = fabs(gx) + fabs(gy)
    write_imagef(dstImg, (int4)(x, y, z, 1), (float4)(g.x, g.y, g.z, 1.0f));
}

__kernel 
//__attribute__((reqd_work_group_size(256, 256, 1)))
void thresholdAndCopy(__read_only image3d_t srcImg,
                    __write_only image3d_t dstImg,
                    sampler_t sampler,
                    int width, int height, int depth)
{
    //w is ignored? I believe w is included as all data types are a power of 2
    int4 startImageCoord = (int4) (get_global_id(0) - 1,
                                   get_global_id(1) - 1,
                                   get_global_id(2) - 1, 
                                   1);
    
    int4 endImageCoord   = (int4) (get_global_id(0) + 1,
                                   get_global_id(1) + 1, 
                                   //removed plus 1 to get indexing proper
                                   get_global_id(2)/* + 1*/, 
                                   1);
    
    int4 outImageCoord = (int4) (get_global_id(0),
                                 get_global_id(1),
                                 get_global_id(2), 
                                 1);
    
    if (outImageCoord.x < width && outImageCoord.y < height && outImageCoord.z < depth)
    {
        float4 thisIn = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        
        //allocate spare buffer
        //float4 tmpBuffer[(endImageCoord.x*endImageCoord.y) - (startImageCoord.x*startImageCoord.y)];
        
        //int rowPitch = endImageCoord.x - startImageCoord.x;
        //int slicePitch = endImageCoord.y - startImageCoord.y;

        
        for(int z = startImageCoord.z; z <= endImageCoord.z; z++)
        {
            //The outer loop is used to process all slices
            
            //forward pass
            for(int y = startImageCoord.y; y <= endImageCoord.y; y++)
            {
                for(int x= startImageCoord.x; x <= endImageCoord.x; x++)
                {
                    
                    thisIn = read_imagef(srcImg, sampler, (int4)(x, y, z, 1));
                    
                    if(thisIn.x > 0.05 || thisIn.y > 0.05 || thisIn.z > 0.05){
                        write_imagef(dstImg, outImageCoord, thisIn);
                    }
                }
            }
            
            //backward pass
//            for(int y = endImageCoord.y; y > startImageCoord.y; y--)
//            {
//                for(int x= endImageCoord.x; x > startImageCoord.x; x--)
//                {
//                    
//                    thisIn = tmpBuffer[(y*rowPitch)+x];
//                    
//                    // Write the output value to image
//                    write_imagef(dstImg, outImageCoord, thisIn);
//                    
//                }
//            }
        }
    }
    
}

__kernel 
// kernel simply copies from input buffer to output
void straightCopy(__read_only image3d_t srcImg,
          __write_only image3d_t dstImg,
          sampler_t sampler,
          int width, int height, int depth)
{
    //w is ignored? I believe w is included as all data types are a power of 2
    int4 startImageCoord = (int4) (get_global_id(0) - 1,
                                   get_global_id(1) - 1,
                                   get_global_id(2) - 1, 
                                   1);
    
    int4 endImageCoord   = (int4) (get_global_id(0) + 1,
                                   get_global_id(1) + 1, 
                                   //removed plus 1 to get indexing proper
                                   get_global_id(2)/* + 1*/, 
                                   1);
    
    int4 outImageCoord = (int4) (get_global_id(0),
                                 get_global_id(1),
                                 get_global_id(2), 
                                 1);
    
    if (outImageCoord.x < width && outImageCoord.y < height && outImageCoord.z < depth){
        for(int z = startImageCoord.z; z <= endImageCoord.z; z++){
            for(int y = startImageCoord.y; y <= endImageCoord.y; y++){
                for(int x= startImageCoord.x; x <= endImageCoord.x; x++){
                    write_imagef(dstImg, outImageCoord, read_imagef(srcImg, sampler, (int4)(x, y, z, 1)));
                }
            }
        }
    }
    
}


