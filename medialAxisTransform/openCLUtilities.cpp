//
//  openCLUtilities.c
//  openCLImageLoad
//
//  Created by Beau Johnston on 17/06/11.
//  Copyright 2011 University Of New England. All rights reserved.
//

#include "openCLUtilities.h"


char *print_cl_errstring(cl_int err) {
    switch (err) {
        case CL_SUCCESS:                          return strdup("Success!");
        case CL_DEVICE_NOT_FOUND:                 return strdup("Device not found.");
        case CL_DEVICE_NOT_AVAILABLE:             return strdup("Device not available");
        case CL_COMPILER_NOT_AVAILABLE:           return strdup("Compiler not available");
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return strdup("Memory object allocation failure");
        case CL_OUT_OF_RESOURCES:                 return strdup("Out of resources");
        case CL_OUT_OF_HOST_MEMORY:               return strdup("Out of host memory");
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return strdup("Profiling information not available");
        case CL_MEM_COPY_OVERLAP:                 return strdup("Memory copy overlap");
        case CL_IMAGE_FORMAT_MISMATCH:            return strdup("Image format mismatch");
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return strdup("Image format not supported");
        case CL_BUILD_PROGRAM_FAILURE:            return strdup("Program build failure");
        case CL_MAP_FAILURE:                      return strdup("Map failure");
        case CL_INVALID_VALUE:                    return strdup("Invalid value");
        case CL_INVALID_DEVICE_TYPE:              return strdup("Invalid device type");
        case CL_INVALID_PLATFORM:                 return strdup("Invalid platform");
        case CL_INVALID_DEVICE:                   return strdup("Invalid device");
        case CL_INVALID_CONTEXT:                  return strdup("Invalid context");
        case CL_INVALID_QUEUE_PROPERTIES:         return strdup("Invalid queue properties");
        case CL_INVALID_COMMAND_QUEUE:            return strdup("Invalid command queue");
        case CL_INVALID_HOST_PTR:                 return strdup("Invalid host pointer");
        case CL_INVALID_MEM_OBJECT:               return strdup("Invalid memory object");
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return strdup("Invalid image format descriptor");
        case CL_INVALID_IMAGE_SIZE:               return strdup("Invalid image size");
        case CL_INVALID_SAMPLER:                  return strdup("Invalid sampler");
        case CL_INVALID_BINARY:                   return strdup("Invalid binary");
        case CL_INVALID_BUILD_OPTIONS:            return strdup("Invalid build options");
        case CL_INVALID_PROGRAM:                  return strdup("Invalid program");
        case CL_INVALID_PROGRAM_EXECUTABLE:       return strdup("Invalid program executable");
        case CL_INVALID_KERNEL_NAME:              return strdup("Invalid kernel name");
        case CL_INVALID_KERNEL_DEFINITION:        return strdup("Invalid kernel definition");
        case CL_INVALID_KERNEL:                   return strdup("Invalid kernel");
        case CL_INVALID_ARG_INDEX:                return strdup("Invalid argument index");
        case CL_INVALID_ARG_VALUE:                return strdup("Invalid argument value");
        case CL_INVALID_ARG_SIZE:                 return strdup("Invalid argument size");
        case CL_INVALID_KERNEL_ARGS:              return strdup("Invalid kernel arguments");
        case CL_INVALID_WORK_DIMENSION:           return strdup("Invalid work dimension");
        case CL_INVALID_WORK_GROUP_SIZE:          return strdup("Invalid work group size");
        case CL_INVALID_WORK_ITEM_SIZE:           return strdup("Invalid work item size");
        case CL_INVALID_GLOBAL_OFFSET:            return strdup("Invalid global offset");
        case CL_INVALID_EVENT_WAIT_LIST:          return strdup("Invalid event wait list");
        case CL_INVALID_EVENT:                    return strdup("Invalid event");
        case CL_INVALID_OPERATION:                return strdup("Invalid operation");
        case CL_INVALID_GL_OBJECT:                return strdup("Invalid OpenGL object");
        case CL_INVALID_BUFFER_SIZE:              return strdup("Invalid buffer size");
        case CL_INVALID_MIP_LEVEL:                return strdup("Invalid mip-map level");
        default:                                  return strdup("Unknown");
    }
}

cl_bool there_was_an_error(cl_int err){
    if (err != CL_SUCCESS){
        printf("%s\n", print_cl_errstring(err));
        return 1;
    }
    return 0;
}

cl_bool doesGPUSupportImageObjects(cl_device_id device_id){
    // Make sure the device supports images, otherwise exit 
    cl_bool imageSupport = CL_FALSE; 
    clGetDeviceInfo(device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &imageSupport, NULL); 
    if (imageSupport != CL_TRUE){
        printf("OpenCL device does not support images.");
    }
    return imageSupport;
}

void getGPUUnitSupportedImageFormats(cl_context context){
    
    cl_image_format supported_image_formats[1000];
    cl_uint supported_image_format_list_size;
    
    //collect supported image formats
    cl_int status = clGetSupportedImageFormats(
                                               context,
                                               CL_MEM_READ_ONLY,
                                               CL_MEM_OBJECT_IMAGE3D,
                                               sizeof(supported_image_formats) / sizeof(supported_image_formats[0]),
                                               supported_image_formats,
                                               &supported_image_format_list_size);
    if (status != CL_SUCCESS) {
        printf("%s\n", print_cl_errstring(status));
		exit(1);        
    }
    for (int i = 0; i < supported_image_format_list_size; i++) {
        printf("Supported image format: ");
        switch (supported_image_formats[i].image_channel_order) {
            case CL_R:
                printf("CL_R");
                break;
            case CL_A:
                printf("CL_A");
                break;
            case CL_INTENSITY:
                printf("CL_INTENSITY");
                break;
            case CL_LUMINANCE:
                printf("CL_LUMINANCE");
                break;
            case CL_RG:
                printf("CL_RG");
                break;
            case CL_RA:
                printf("CL_RA");
                break;
            case CL_RGB:
                printf("CL_RGB");
                break;
            case CL_RGBA:
                printf("CL_RGBA");
                break;
            case CL_ARGB:
                printf("CL_ARGB");
                break;
            case CL_BGRA:
                printf("CL_BGRA");
                break;
            default:
                printf("Unknown");
                break;
        }
        printf(", ");
        switch (supported_image_formats[i].image_channel_data_type) {
            case CL_UNORM_INT8:
                printf("CL_UNORM_INT8\n");
                break;
            case CL_UNORM_INT16:
                printf("CL_UNORM_INT16\n");
                break;
            case CL_SNORM_INT8:
                printf("CL_SNORM_INT8\n");
                break;
            case CL_SNORM_INT16:
                printf("CL_SNORM_INT16\n");
                break;
            case CL_HALF_FLOAT:
                printf("CL_HALF_FLOAT\n");
                break;
            case CL_FLOAT:
                printf("CL_FLOAT\n");
                break;
            case CL_UNORM_SHORT_565:
                printf("CL_UNORM_SHORT_565\n");
                break;
            case CL_UNORM_SHORT_555:
                printf("CL_UNORM_SHORT_555\n");
                break;
            case CL_UNORM_INT_101010:
                printf("CL_UNORM_INT_101010\n");
                break;
            case CL_SIGNED_INT8:
                printf("CL_SIGNED_INT8\n");
                break;
            case CL_UNSIGNED_INT8:
                printf("CL_UNSIGNED_INT8\n");
                break;
            case CL_SIGNED_INT16:
                printf("CL_SIGNED_INT16\n");
                break;
            case CL_SIGNED_INT32:
                printf("CL_SIGNED_INT32\n");
                break;
            case CL_UNSIGNED_INT16:
                printf("CL_UNSIGNED_INT16\n");
                break;
            case CL_UNSIGNED_INT32:
                printf("CL_UNSIGNED_INT32\n");
                break;
            default:
                printf("Unknown\n");
                break;
        }
    }
    
}

char *load_program_source(const char *filename)
{
    struct stat statbuf;
    FILE        *fh;
    char        *source;
	
    fh = fopen(filename, "r");
    if (fh == 0)
        return 0;
	
    stat(filename, &statbuf);
    source = (char *) malloc((unsigned long)statbuf.st_size + 1);
    fread(source, (unsigned long)statbuf.st_size, 1, fh);
    source[statbuf.st_size] = '\0';
	
    return source;
}

cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height, cl_image_format &format)
{ 
    read_png_file(fileName);
    
    width = getImageWidth();
    height = getImageLength();
    
    uint8 *buffer = new uint8[getImageSize()];    
    
    //
    // As a Normalized float
    //printImage((uint8*)upcastToFloatAndNormalize(getImage(), getImageSize()), getImageSize());
    
    //
    // Done and Undone
    // printImage(downcastToByteAndDenormalize(upcastToFloatAndNormalize(getImage(), getImageSize()), getImageSize()), getImageSize());

    memcpy(buffer, getImage(), getImageSize());

//    memcpy(buffer, upcastToFloatAndNormalize(getImage(), getImageSize()), getImageSizeInFloats());

    //
    // With this buffer
    //printImage(downcastToByteAndDenormalize((float*)buffer, getImageSizeInFloats()), getImageSize());
    
    //test with red sample image
//    setImage(createRedTile());
//    
//    width=10;
//    height=10;
//    uint8*buffer = new uint8[10*10*4*sizeof(float)];
//    memcpy(buffer, upcastToFloatAndNormalize(getImage(), 10*10), 10*10*4*sizeof(float));
    
    //end of test with sample image
    
    
    
    format.image_channel_order = CL_RGBA; 
    format.image_channel_data_type = CL_UNORM_INT8;
        
    cl_int errNum; 
    cl_mem clImage; 
    
    clImage = clCreateImage2D(context,
                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                              &format, 
                              width,
                              height, 
                              getImageRowPitch(), 
                              buffer, 
                              &errNum);
    
    
    //normalized values in buffer
    //
    //printImage(buffer, getImageSize());
    
    //unnormalized values from buffer
    //
    //printImage(downcastToByteAndDenormalize((float*)buffer, getImageSizeInFloats()), getImageSize());
    
    if (errNum != CL_SUCCESS) {
        printf("Error creating CL image object\n"); 
        return 0;
    }
    return clImage; 
}


bool SaveImage(char *fileName, uint8 *buffer, int width, int height) {
    //setImage((uint8*)denormalizeImage((float*)buffer));
    //write_png_file(fileName);
    
    //printImage(downcastToByteAndDenormalize((float*)buffer, getImageSizeInFloats()), getImageSize());
    
//    setImage(downcastToByteAndDenormalize((float*)buffer, getImageSizeInFloats()));
    setImage(buffer);

    //setImageFromFloat(downcastToByteAndDenormalize((float*)buffer, getImageSize()));
    
    write_png_file(fileName);
    
    return true;
}

//  Round up to the nearest multiple of the group size
//
size_t RoundUp(int groupSize, int globalSize)   {
    
    int r = globalSize % groupSize;
    if(r == 0)
    {
     	return globalSize;
    }
    else
    {
     	return globalSize + groupSize - r;
    }
}

cl_mem LoadStackOfImages(cl_context context, char *fileName, int &width, int &height, int &depth, cl_image_format &format)
{ 
    generateListOfAssociatedFiles(fileName);

    depth = numberOfFiles();
    
    uint8*bigBuffer;
    bool firstRun = true;
    
    //load all images into a buffer
    for (int i = 0; i < numberOfFiles(); i++) {
        read_png_file(getNextFileName());
        width = getImageWidth();
        height = getImageLength();
        uint8 *buffer = new uint8[getImageSize()];
        memcpy(buffer, getImage(), getImageSize());
        if (firstRun) {
            //if its the first run we don't know the dimensions of the image
            //and thus don't know how much memory to statically allocate
            bigBuffer = new uint8[getImageSize()*depth];
            firstRun = false;
        }
        memcpy(bigBuffer+(i*getImageSize()), buffer, getImageSize());
    } 
    
    //printImage(bigBuffer, getImageSize()*depth);
    
    
    format.image_channel_order = CL_RGBA; 
    format.image_channel_data_type = CL_UNORM_INT8;
    
    cl_int errNum; 
    cl_mem clImage; 
    
    //    cl_mem clCreateImage3D (cl_context context, cl_mem_flags flags,
    //                            const cl_image_format *image_format, size_t image_width,
    //                            size_t image_height,
    //                            size_t image_depth,
    //                            size_t image_row_pitch, size_t image_slice_pitch, void *host_ptr,
    //                            cl_int *errcode_ret)
    clImage = clCreateImage3D(context,
                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                              &format, 
                              width,
                              height,
                              depth,
                              getImageRowPitch(),
                              getImageSlicePitch(),
                              bigBuffer, 
                              &errNum);
    
    
    if (errNum != CL_SUCCESS) {
        printf("Error creating CL image object\n"); 
        return 0;
    }
    return clImage; 
}

size_t getImageSlicePitch(){
    return getImageRowPitch()*getImageLength();
}

