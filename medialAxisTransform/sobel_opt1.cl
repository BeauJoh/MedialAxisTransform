#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void sobel(__read_only image3d_t srcImg,
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
        float4 outColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        for(int z = startImageCoord.z; z <= endImageCoord.z; z++)
        {
            for(int y = startImageCoord.y; y <= endImageCoord.y; y++)
            {
                for(int x= startImageCoord.x; x <= endImageCoord.x; x++)
                {
                    outColor = read_imagef(srcImg, sampler, (int4)(x, y, z, 1));
                                    
                    // Write the output value to image
                    write_imagef(dstImg, outImageCoord, outColor);
                }
            }   
        }
    }
    
}
