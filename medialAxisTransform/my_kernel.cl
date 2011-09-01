//
//  myKernel.h
//  MedialAxisTransform
//
//  Created by Beau Johnston on 25/08/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


float4 getFillColour(){
    return (float4) (0.5f,0.5f,0.5f,1);
}

/*
 This computes an in-place complex-to-complex FFT 
 x and y are the real and imaginary arrays of 2^m points.
 dir =  1 gives forward transform
 dir = -1 gives reverse transform 
 */
void FFT(short int dir,long m, float *x,float *y)
{
    long n,i,i1,j,k,i2,l,l1,l2;
    double c1,c2,tx,ty,t1,t2,u1,u2,z;
    
    /* Calculate the number of points */
    n = 1;
    for (i=0;i<m;i++) 
        n *= 2;
    
    /* Do the bit reversal */
    i2 = n >> 1;
    j = 0;
    for (i=0;i<n-1;i++) {
        if (i < j) {
            tx = x[i];
            ty = y[i];
            x[i] = x[j];
            y[i] = y[j];
            x[j] = tx;
            y[j] = ty;
        }
        k = i2;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    
    /* Compute the FFT */
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
                x[i1] = x[i] - t1; 
                y[i1] = y[i] - t2;
                x[i] += t1;
                y[i] += t2;
            }
            z =  u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = sqrt((1.0 - c1) / 2.0);
        if (dir == 1) 
            c2 = -c2;
        c1 = sqrt((1.0 + c1) / 2.0);
    }
    
    /* Scaling for forward transform */
    if (dir == 1) {
        for (i=0;i<n;i++) {
            x[i] /= n;
            y[i] /= n;
        }
    }
    
    return;
}

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
    
    
    //rest goes here!
    
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
void mrep(__read_only image3d_t srcImg,
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
                        write_imagef(dstImg, outImageCoord, getFillColour());
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

