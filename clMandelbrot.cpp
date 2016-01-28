
#ifndef _WIN32
#include <sys/time.h>
#else
#include <Windows.h>
#endif


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// OpenCL includes
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

// draw bmp
struct BMPHeader
{
    char bfType[2];       /* "BM" */
    int bfSize;           /* Size of file in bytes */
    int bfReserved;       /* set to 0 */
    int bfOffBits;        /* Byte offset to actual bitmap data (= 54) */
    int biSize;           /* Size of BITMAPINFOHEADER, in bytes (= 40) */
    int biWidth;          /* Width of image, in pixels */
    int biHeight;         /* Height of images, in pixels */
    short biPlanes;       /* Number of planes in target device (set to 1) */
    short biBitCount;     /* Bits per pixel (24 in this case) */
    int biCompression;    /* Type of compression (0 if no compression) */
    int biSizeImage;      /* Image size, in bytes (0 if no compression) */
    int biXPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biYPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biClrUsed;        /* Number of colors in the color table (if 0, use
                             maximum allowed by biBitCount) */
    int biClrImportant;   /* Number of important colors.  If 0, all colors
                             are important */
};

int write_bmp(const char *filename, int width, int height, unsigned short *rgb)
{
    int i, j, ipos;
    int bytesPerLine;
    unsigned char *line;

    FILE *file;
    struct BMPHeader bmph;

    // The length of each line must be a multiple of 4 bytes

    bytesPerLine = (3 * (width + 1) / 4) * 4;

    strncpy(bmph.bfType, "BM", 2);
    bmph.bfOffBits = 54;
    bmph.bfSize = bmph.bfOffBits + bytesPerLine * height;
    bmph.bfReserved = 0;
    bmph.biSize = 40;
    bmph.biWidth = width;
    bmph.biHeight = height;
    bmph.biPlanes = 1;
    bmph.biBitCount = 24;
    bmph.biCompression = 0;
    bmph.biSizeImage = bytesPerLine * height;
    bmph.biXPelsPerMeter = 0;
    bmph.biYPelsPerMeter = 0;
    bmph.biClrUsed = 0;
    bmph.biClrImportant = 0;

    file = fopen (filename, "wb");
    if (file == NULL) return(0);

    fwrite(&bmph.bfType, 2, 1, file);
    fwrite(&bmph.bfSize, 4, 1, file);
    fwrite(&bmph.bfReserved, 4, 1, file);
    fwrite(&bmph.bfOffBits, 4, 1, file);
    fwrite(&bmph.biSize, 4, 1, file);
    fwrite(&bmph.biWidth, 4, 1, file);
    fwrite(&bmph.biHeight, 4, 1, file);
    fwrite(&bmph.biPlanes, 2, 1, file);
    fwrite(&bmph.biBitCount, 2, 1, file);
    fwrite(&bmph.biCompression, 4, 1, file);
    fwrite(&bmph.biSizeImage, 4, 1, file);
    fwrite(&bmph.biXPelsPerMeter, 4, 1, file);
    fwrite(&bmph.biYPelsPerMeter, 4, 1, file);
    fwrite(&bmph.biClrUsed, 4, 1, file);
    fwrite(&bmph.biClrImportant, 4, 1, file);

    line = (unsigned char*)malloc(bytesPerLine);
    if (line == NULL)
    {
        fprintf(stderr, "Can't allocate memory for BMP file.\n");
        return(0);
    }

    for (i = height - 1; i >= 0; i--)
    {
        for (j = 0; j < width; j++)
        {
            ipos = width * i + j;

			// Simple color, black and white            
			//line[3*j] = rgb[ipos];
            //line[3*j+1] = rgb[ipos];
            //line[3*j+2] = rgb[ipos];

			// Color algorithm start
			int c = rgb[ipos];
			if ( c == 0)
			{
				// inside the Mandelbrot set
				line[3*j] = 0;
				line[3*j + 1] = 0;
				line[3*j + 2] = 0;
			} 
			else
			{
				float s = 3*logf(c)/logf(65535);
				if (s < 1)
				{
					line[3*j] = 255 * s;
					line[3*j + 1] = 0;
					line[3*j + 2] = 0;
				}
				else if (s < 2)	
				{
					line[3*j] = 255;
					line[3*j + 1] = 255 * (s - 1);
					line[3*j + 2] = 0;
				}
				else				
				{
					line[3*j] = 255;
					line[3*j + 1] = 255;
					line[3*j + 2] = 255 * (s -2);
				}
			}
			// Color algorithm end
        }
        fwrite(line, bytesPerLine, 1, file);
    }

    free(line);
    fclose(file);

    return(1);
}

// OpenCL kernel to calculate mandelbrot set 
const char* programSource =
"__kernel                                              	\n"
"void mandel(__global unsigned short* output)           \n"
"{                                                     	\n"
"   int x_dim = get_global_id(0);                      	\n"
"   int y_dim = get_global_id(1);                      	\n"
"   size_t w = get_global_size(0);                     	\n"
"   size_t h = get_global_size(1);                     	\n"
"   int index = y_dim*w + x_dim;                       	\n"
"   float x_seed = (x_dim/(float)w)*2.5f - 2;          	\n"
"   float y_seed = ((y_dim+w-h)/(float)w)*2 -1;         \n"
"   float x = 0.0;                                      \n"
"	float y = 0.0;                                      \n"
"	int iteration = 0;	                                \n"	
"   int max = 65535;                      	            \n"
"   do {                                                \n"
"       iteration++;                                    \n"
"       if (iteration > max) break;                     \n"
"       float xtemp = x;                                \n"
"       x = x*x - y*y + x_seed;                         \n"	
"       y = 2*xtemp*y + y_seed;	   	   	                \n"
"      }                                                \n"
"   while (x*x + y*y <= 4);	       	                    \n"
"	//printf(\"\\n%d, %d\", index, iteration);          \n"
"	output[index] = (iteration > max)?0:iteration;      \n"
"}                                                     	\n"
;


int main(int argc, char **argv) {
    int r,c;
	int W, H, p;

	if (argc == 3) {
		W = atoi(argv[1]);
		p = atoi(argv[2]);
	}
	else if (argc == 2) {
		W = atoi(argv[1]);
		p = 4; // all computataion on server
	}
	else {
		W = 1024;
		p = 4;
	}
	
	// p = 0, all computation on client, illegal argument, client should have never send request to server.
	// p = 1, bottom 25% of mandelbrot set (bottom 25% of the picture) is computed on server.
	// p = 2, bottom 50% is computed on server.
	// p = 3, bottom 75% is computed on server.
	// p = 4, all is computed on server.

	H = W*p/4; // adjust the height according to the workload split

#ifndef _WIN32
    struct timeval tv, tv2;
    struct timezone tz;
#endif
	int ok, ms;

	unsigned short* color = (unsigned short*)(malloc(W*H*sizeof(unsigned short)));

#ifndef _WIN32
    gettimeofday(&tv, &tz);
#else
	ms = GetTickCount();
#endif

    {    		
		//setup opencl host
		// Use this to check the output of each API call
		cl_int status;  
     
		// Retrieve the number of platforms
		cl_uint numPlatforms = 0;
		status = clGetPlatformIDs(0, NULL, &numPlatforms);
 
		// Allocate enough space for each platform
		cl_platform_id *platforms = NULL;
		platforms = (cl_platform_id*)malloc(
			numPlatforms*sizeof(cl_platform_id));
 
		// Fill in the platforms
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);

		// Retrieve the number of devices
		cl_uint numDevices = 0;
		status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, 
			NULL, &numDevices);

		// Allocate enough space for each device
		cl_device_id *devices;
		devices = (cl_device_id*)malloc(
			numDevices*sizeof(cl_device_id));

		// Fill in the devices 
		status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL,        
			numDevices, devices, NULL);

		// Create a context and associate it with the devices
		cl_context context;
		context = clCreateContext(NULL, numDevices, devices, NULL, 
			NULL, &status);

		// Create a command queue and associate it with the device 
		cl_command_queue cmdQueue;
		cmdQueue = clCreateCommandQueue(context, devices[0], 0, 
			&status);

		const int elements = W*H;
		size_t datasize = sizeof(unsigned short)*elements;

		// Create a buffer object that will hold the output data
		cl_mem output;
		output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize,
			NULL, &status);

		// Create a program with source code
		cl_program program = clCreateProgramWithSource(context, 1, 
			(const char**)&programSource, NULL, &status);

		// Build (compile) the program for the device
		status = clBuildProgram(program, numDevices, devices, 
			NULL, NULL, NULL);

		if (status != CL_SUCCESS)
		{
			printf("build program status = %d\n", status);
			size_t len;
			char buffer[2048];
			clGetProgramBuildInfo(program, *devices, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			printf("Build Log: \n%s\n", buffer);
		}

		// Create the mandelbrot kernel
		cl_kernel kernel;
		kernel = clCreateKernel(program, "mandel", &status);

		// Associate the input and output buffers with the kernel 
		status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &output);

		/* Execute the OpenCL kernel */
		size_t global_item_size[2] = {W, H};
		//size_t local_item_size[2] = {W/2, H/2};
		// Let the OpenCL runtime to set the local item size
		status = clEnqueueNDRangeKernel	(cmdQueue, kernel, 2, NULL,
            global_item_size, /*local_item_size*/NULL, 0, NULL, NULL);

		// Read the device output buffer to the host output array
		clEnqueueReadBuffer(cmdQueue, output, CL_TRUE, 0, 
			datasize, color, 0, NULL, NULL);
		status = clFlush(cmdQueue);
		status = clFinish(cmdQueue);

		// Free OpenCL resources
		clReleaseKernel(kernel);
		clReleaseProgram(program);
		clReleaseCommandQueue(cmdQueue);
		clReleaseMemObject(output);
		clReleaseContext(context);

		 // Free host resources
		free(platforms);
		free(devices);
    }

#ifndef _WIN32
    gettimeofday(&tv2, &tz);

    ms = (tv2.tv_sec - tv.tv_sec) * 1000000 + tv2.tv_usec - tv.tv_usec;
    printf("\nComputing time in us: %d\n", ms);
#else
	ms = GetTickCount() - ms;
    printf("\nComputing time in ms: %d\n", ms);
#endif

	write_bmp("output.bmp", W, H, color);
	
	FILE* file = fopen ("output.bin", "wb");
	fwrite (color, 2, W*H, file);
	fclose (file);
}
