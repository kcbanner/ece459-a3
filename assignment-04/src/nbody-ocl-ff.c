/* nbody simulation, version 0 */
/* Modified by Patrick Lam; original source: GPU Gems, Chapter 31 */

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/// runtime on my 2011 computer: 1m; in 2013, 27s.
// on my 2011 laptop, 1m34s
#define POINTS 500 * 64
#define SPACE 1000.0f
#define BINS 1000

#define POSITION_BUFFER 0
#define ACCELLERATION_BUFFER 1
#define BUCKETS 1


cl_float4 * initializePositions() {
    cl_float4 * pts = (cl_float4*) malloc(sizeof(cl_float4)*POINTS);
    int i;

    srand(42L); // for deterministic results

    for (i = 0; i < POINTS; i++) {
        // quick and dirty generation of points
        // not random at all, but I don't care.
        pts[i].x = ((float)rand())/RAND_MAX * SPACE;
        pts[i].y = ((float)rand())/RAND_MAX * SPACE;
        pts[i].z = ((float)rand())/RAND_MAX * SPACE;
        pts[i].w = 1.0f; // size = 1.0f for simplicity.
    }
    return pts;
}


std::string* loadKernelSource(std::string source) {
  std::ifstream file(source.c_str()); 
  std::string* contents = new std::string((std::istreambuf_iterator<char>(file)),
                                          (std::istreambuf_iterator<char>()));

  return contents;
}


cl_float4* computeBins(cl::Context& context, cl::CommandQueue& queue, cl::Program& program, cl_float4* x)
{

    // Make kernel
    cl::Kernel kernel(program, "bin");

    // Create memory buffers
    cl::Buffer position_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, POINTS * sizeof(cl_float4));
    cl::Buffer bins_buffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, BINS * sizeof(cl_float4));

    // Copy lists the memory buffers
    queue.enqueueWriteBuffer(position_buffer, CL_TRUE, 0, POINTS * sizeof(cl_float4), x);
    
    // Set arguments to kernel
    //int buffer_size = POINTS;
    //kernel.setArg(BUFFER_SIZE, buffer_size);
    kernel.setArg(POSITION_BUFFER, position_buffer);
    kernel.setArg(BUCKETS, bins_buffer);

    // Run the kernel on specific ND range
    cl::NDRange global(BINS);
    cl::NDRange local(1);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);

    // Read buffer C into a local list
    cl_float4* bins = new cl_float4[BINS];
    queue.enqueueReadBuffer(bins_buffer, CL_TRUE, 0, BINS * sizeof(cl_float4), bins);

    return bins;  
}

unsigned int* sortBins(cl::Context& context, cl::CommandQueue& queue, cl::Program& program,
                    cl_float4* x, unsigned int* offsets)
{
    int arraySize = POINTS*sizeof(unsigned int);

    // Make kernel
    cl::Kernel kernel(program, "sort");

    // Create memory buffers

    cl::Buffer position_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, POINTS * sizeof(cl_float4));
    cl::Buffer offsets_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, BINS*sizeof(unsigned int));
    cl::Buffer bins_buffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, arraySize);

    // Copy lists the memory buffers
    queue.enqueueWriteBuffer(position_buffer, CL_TRUE, 0, POINTS * sizeof(cl_float4), x);
    queue.enqueueWriteBuffer(offsets_buffer, CL_TRUE, 0, BINS * sizeof(unsigned int), offsets);
    
    // Set arguments to kernel
    kernel.setArg(0, position_buffer);
    kernel.setArg(1, offsets_buffer);
    kernel.setArg(2, bins_buffer);

    // Run the kernel on specific ND range
    cl::NDRange global(BINS);
    cl::NDRange local(1);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);

    // Read buffer C into a local list
    unsigned int* sortedBins = new unsigned int[POINTS];
    queue.enqueueReadBuffer(bins_buffer, CL_TRUE, 0, arraySize, sortedBins);

    return sortedBins;
}


cl_float4* forces(cl::Context& context, cl::CommandQueue& queue, cl::Program& program,
                    cl_float4* x, cl_float4* cm, unsigned int* offsets, unsigned int* bins)
{
    cl::Kernel kernel(program, "forces");

    // Create memory buffers

    cl::Buffer position_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, POINTS * sizeof(cl_float4));
    cl::Buffer cm_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, BINS * sizeof(cl_float4));
    cl::Buffer offsets_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, BINS*sizeof(unsigned int));
    cl::Buffer bins_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, POINTS*sizeof(unsigned int));
    cl::Buffer forces_buffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, POINTS*sizeof(cl_float4));

    
    // Copy lists the memory buffers
    queue.enqueueWriteBuffer(position_buffer, CL_TRUE, 0, POINTS * sizeof(cl_float4), x);
    queue.enqueueWriteBuffer(cm_buffer, CL_TRUE, 0, BINS * sizeof(cl_float4), cm);
    queue.enqueueWriteBuffer(offsets_buffer, CL_TRUE, 0, BINS * sizeof(unsigned int), offsets);
    queue.enqueueWriteBuffer(bins_buffer, CL_TRUE, 0, POINTS * sizeof(unsigned int), bins);
    
    // Set arguments to kernel
    //int buffer_size = POINTS;
    //kernel.setArg(BUFFER_SIZE, buffer_size);
    kernel.setArg(0, position_buffer);
    kernel.setArg(1, cm_buffer);
    kernel.setArg(2, offsets_buffer);
    kernel.setArg(3, bins_buffer);
    kernel.setArg(4, forces_buffer);

    // Run the kernel on specific ND range
    cl::NDRange global(POINTS);
    cl::NDRange local(1);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);

    // Read buffer C into a local list
    cl_float4* forces = new cl_float4[POINTS];
    queue.enqueueReadBuffer(forces_buffer, CL_TRUE, 0, POINTS*sizeof(cl_float4), forces);

    return forces;
}

int main(int argc, char ** argv)
{
    cl_float4 * x = initializePositions();

    std::string* source = loadKernelSource("src/kernel.cl");

    // Get available platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] = { 
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)(platforms[0])(), 
        0 
    };
    cl::Context context(CL_DEVICE_TYPE_GPU, cps);

    // Get a list of devices on this platform
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    // Create a command queue and use the first device
    cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

    cl::Program::Sources sources(1, std::make_pair(source->c_str(), source->length()+1));

    // Make program of the source code in the context
    cl::Program program = cl::Program(context, sources);

    // Build program for these specific devices
    try{
        program.build(devices);

    } catch(cl::Error error) {
        std::cout << error.what() << "(" << error.err() << ")" << std::endl;

        std::string build_log;
        build_log = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]);
        std::cout << "Build status: " << build_log << std::endl;

        build_log = program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0]);
        std::cout << "Build options: " << build_log << std::endl;

        build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
        std::cout << "Build log: " << build_log << std::endl;
        exit(0);
    }


    //runKernel(context, queue, devices, *source, x);
    cl_float4* cm = computeBins(context, queue, program, x);
    int cumsum = 0;
    for(int i = 0; i < BINS; i++) {
        cumsum += (int) cm[i].w;
    }

    unsigned int* offsets = (unsigned int*) malloc(sizeof(int)*BINS);

    offsets[0] = 0;
    for(int i =1; i < BINS; i++) {
        offsets[i] = offsets[i-1] + cm[i-1].w;
    }

    unsigned int* sortedBins = sortBins(context, queue, program, x, offsets);
    cl_float4* a = forces(context, queue, program, x, cm, offsets, sortedBins);

    for(int i = 0; i < POINTS; i++){
        printf("(%2.2f,%2.2f,%2.2f,%2.2f) (%2.3f,%2.3f,%2.3f)\n", 
                   x[i].x, x[i].y, x[i].z, x[i].w,
                   a[i].x, a[i].y, a[i].z);
    }
    free(x);
    return 0;
}
