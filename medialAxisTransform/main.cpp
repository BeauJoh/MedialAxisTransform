
#include "openCLUtilities.h"
#include "RGBAUtilities.h"

#include <getopt.h>
#include <string>
#include <iostream>
using namespace std;

// getopt argument parser variables
string imageFileName;
string kernelFileName;
string outputImageFileName;

// OpenCL variables
int err, gpu;                       // error code returned from api calls
size_t *globalWorksize;             // global domain size for our calculation
size_t *localWorksize;              // local domain size for our calculation
cl_device_id device_id;             // compute device id 
cl_context context;                 // compute context
cl_command_queue commands;          // compute command queue
cl_program program;                 // compute program
cl_kernel kernel;                   // compute kernel
cl_sampler sampler;
cl_mem input;                       // device memory used for the input array
cl_mem output;                      // device memory used for the output array
int width;
int height;                         //input and output image specs
int depth;


static inline int parseCommandLine(int argc , char** argv){
    {
        int c;
        while (true)
        {
            static struct option long_options[] =
            {
                /* These options don't set a flag.
                 We distinguish them by their indices. */
                {"kernel",required_argument,       0, 'k'},
                {"image",  required_argument,       0, 'i'},
                {"output-image", required_argument, 0, 'o'},
                {0, 0, 0, 0}
            };
            /* getopt_long stores the option index here. */
            int option_index = 0;
            
            c = getopt_long (argc, argv, ":k:i:o:",
                             long_options, &option_index);
            
            /* Detect the end of the options. */
            if (c == -1)
                break;
            
            switch (c)
            {
                case 0:
                    /* If this option set a flag, do nothing else now. */
                    if (long_options[option_index].flag != 0)
                        break;
                    printf ("option %s", long_options[option_index].name);
                    if (optarg)
                        printf (" with arg %s", optarg);
                    printf ("\n");
                    break;
                    
                case 'i':
                    imageFileName = optarg ;
                    break;
                    
                case 'o':
                    outputImageFileName = optarg;
                    break;
                    
                case 'k':
                    kernelFileName = optarg ;
                    break;
                    
                    
                case '?':
                    /* getopt_long already printed an error message. */
                    break;
                    
                default:
                    ;
                    
            }
        }
        
        
        /* Print any remaining command line arguments (not options). */
        if (optind < argc)
        {
            while (optind < argc)
            /*
             printf ("%s ", argv[optind]);
             putchar ('\n');
             */
                optind++;
        }
    }
    return 1;
};


void cleanKill(int errNumber){
    clReleaseMemObject(input);
	clReleaseMemObject(output);
	clReleaseProgram(program);
    clReleaseSampler(sampler);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);
    exit(errNumber);
}

int main(int argc, char *argv[])
{	
    parseCommandLine(argc , argv);

	// Connect to a compute device
	//
	gpu = 1;
	if(clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL) != CL_SUCCESS)  {
		cout << "Failed to create a device group!" << endl;
        cleanKill(EXIT_FAILURE);
    }
	
	// Create a compute context 
	//
	if(!(context = clCreateContext(0, 1, &device_id, NULL, NULL, &err))){
		cout << "Failed to create a compute context!" << endl;
        cleanKill(EXIT_FAILURE);
    }
	
	// Create a command commands
	//
	if(!(commands = clCreateCommandQueue(context, device_id, 0, &err))) {
		cout << "Failed to create a command commands!" << endl;
        cleanKill(EXIT_FAILURE);
    }
    
    // Load kernel source code
    //
    //	char *source = load_program_source((char*)"sobel_opt1.cl");
    char *source = load_program_source((char*)kernelFileName.c_str());
    if(!source)
    {
        cout << "Error: Failed to load compute program from file!" << endl;
        cleanKill(EXIT_FAILURE);
    }
    
	// Create the compute program from the source buffer
	//
	if(!(program = clCreateProgramWithSource(context, 1, (const char **) &source, NULL, &err))){
		cout << "Failed to create compute program!" << endl;
        cleanKill(EXIT_FAILURE);
    }
    
	// Build the program executable
	//
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		cout << "Error: Failed to build program executable!" << endl;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		cout << buffer << endl;
        cleanKill(EXIT_FAILURE);
	}
    
    if(!doesGPUSupportImageObjects){
        cleanKill(EXIT_FAILURE);
    }
	
	// Create the compute kernel in the program we wish to run
	//
	kernel = clCreateKernel(program, "sobel", &err);
    
	if (!kernel || err != CL_SUCCESS){
		cout << "Failed to create compute kernel!" << endl;
        cleanKill(EXIT_FAILURE);
    }
    
    // Get GPU image support, useful for debugging
    // getGPUUnitSupportedImageFormats(context);
    
    
    //  specify the image format that the images are represented as... 
    //  by default to support OpenCL they must support 
    //  format.image_channel_data_type = CL_UNORM_INT8;
    //  i.e. Each channel component is a normalized unsigned 8-bit integer value.
    //
    //	format.image_channel_order = CL_RGBA;
    //
    //  format is collected with the LoadImage function
    cl_image_format format; 

    //  create input image buffer object to read results from
    input = LoadStackOfImages(context, (char*)imageFileName.c_str(), width, height, depth, format);
    
    //  create output buffer object, to store results
    output = clCreateImage3D(context, 
                             CL_MEM_WRITE_ONLY, 
                             &format, 
                             width, 
                             height,
                             depth,
                             getImageRowPitch(), 
                             getImageSlicePitch(),
                             NULL, 
                             &err);

    if(there_was_an_error(err)){
        cout << "Output Image Buffer creation error!" << endl;
        cleanKill(EXIT_FAILURE);
    }    
    
    
    //  if either input of output are empty, crash and burn
	if (!input || !output ){
		cout << "Failed to allocate device memory!" << endl;
        cleanKill(EXIT_FAILURE);
	}
    
    
    // Create sampler for sampling image object 
    sampler = clCreateSampler(context,
                              CL_FALSE, // Non-normalized coordinates 
                              CL_ADDRESS_CLAMP_TO_EDGE, 
                              CL_FILTER_NEAREST, 
                              &err);
    
    if(there_was_an_error(err)){
        cout << "Error creating CL sampler object." << endl;
        cleanKill(EXIT_FAILURE);
    }
    
    
	// Set the arguments to our compute kernel
	//
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler); 
    err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &width);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &height);
    err |= clSetKernelArg(kernel, 5, sizeof(cl_int), &depth);
    //depth arg here!
    
    if(there_was_an_error(err)){
        cout << "Error: Failed to set kernel arguments! " << err << endl;
        cleanKill(EXIT_FAILURE);
    }    
    
    // THIS IS POORLY DOCUMENTED ELSEWHERE!
    // each independed image object steam needs its own local & global data spec
    // thus while I use 1 input and 1 output object local[0] for local input
    // and local[1] for local output
    
//    localWorksize[0] = 1;
//    localWorksize[1] = localWorksize[0];
//    globalWorksize[0] = width*height;
//    globalWorksize[1] = globalWorksize[0];
    
//    size_t localWorksize[3] = { 16, 16 , 1};
//    size_t globalWorksize[3] =  { RoundUp((int)localWorksize[0], width), RoundUp((int)localWorksize[1], height), RoundUp((int)localWorksize[2], depth) };

    size_t localWorksize[3] = { 1, 1, 1};
    size_t globalWorksize[3] =  { getImageWidth(), getImageHeight(), depth};
    
    //  Start up the kernels in the GPUs
    //
	err = clEnqueueNDRangeKernel(commands, kernel, 3, NULL, globalWorksize, localWorksize, 0, NULL, NULL);
    
	if (there_was_an_error(err))
	{
        cout << print_cl_errstring(err) << endl;
		cout << "Failed to execute kernel!, " << err << endl;
        cleanKill(EXIT_FAILURE);
	}
	
	// Wait for the command commands to get serviced before reading back results
	//
	clFinish(commands);
    
	// Read back the results from the device to verify the output
	//
    uint8* bigBuffer = new uint8[getImageSize()*depth];        
    
    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { width, height, depth};
    
    cl_command_queue queue = clCreateCommandQueue(
                                                  context, 
                                                  device_id, 
                                                  0, 
                                                  &err);
    
    // Read image to buffer with implicit row pitch calculation
    //
    err = clEnqueueReadImage(queue, output,
                             CL_TRUE, origin, region, getImageRowPitch(), getImageSlicePitch(), bigBuffer, 0, NULL, NULL);
    
    
    //printImage(buffer, getImageSize()*depth);
        
    //load all images into a buffer
    for (int i = 0; i < depth; i++) {
        uint8 *buffer = new uint8[getImageSize()];
        memcpy(buffer, bigBuffer+(i*getImageSize()), getImageSize());
        
        string file = outputImageFileName.substr(outputImageFileName.find_last_of('/')+1);
        string path = outputImageFileName.substr(0, outputImageFileName.find_last_of('/')+1);
        
        string cutDownFile = file.substr(0, file.find_last_of('.'));
        string extension = file.substr(file.find_last_of('.'));
        
        
        string newName = cutDownFile;
        char numericalRepresentation[200];
        sprintf(numericalRepresentation, "%d", i);
        newName.append(numericalRepresentation);
        newName.append(extension);
        
        newName = path.append(newName);
        
        cout << newName << endl;

        SaveImage((char*)newName.c_str(), buffer, width, height);   
        
        //clear buffer was originally used as a test to ensure new images 
        //are being populated
        //clearImageBuffer();


    } 
    cleanup();
    
    cout << "RUN FINISHED SUCCESSFULLY!" << endl;
    
    
    
    // Shutdown and cleanup
	//
	cleanKill(EXIT_SUCCESS);
	//return 1;
}