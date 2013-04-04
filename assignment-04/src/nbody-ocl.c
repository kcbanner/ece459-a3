/* nbody simulation, version 0 */
/* Modified by Patrick Lam; original source: GPU Gems, Chapter 31 */

#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define EPS 1e-10

/// runtime on my 2011 computer: 1m; in 2013, 27s.
// on my 2011 laptop, 1m34s
#define POINTS 500 * 64
#define SPACE 1000.0f;

#define BUFFER_SIZE 0
#define POSITION_BUFFER 1
#define ACCELLERATION_BUFFER 2

cl_float4* runKernel(const char* source);

void bodyBodyInteraction(cl_float4 bi, cl_float4 bj, cl_float4 *ai) {
    cl_float4 r;
    
    r.x = bj.x - bi.x;
    r.y = bj.y - bi.y;
    r.z = bj.z - bi.z;
    r.w = 1.0f;

    float distSqr = r.x * r.x + r.y * r.y + r.z * r.z + EPS;

    float distSixth = distSqr * distSqr * distSqr;
    float invDistCube = 1.0f/sqrtf(distSixth);

    float s = bj.w * invDistCube;

    ai->x += r.x * s;
    ai->y += r.y * s;
    ai->z += r.z * s;
}

void calculateForces(int points, int global_id, cl_float4 * globalP, cl_float4 * globalA) {
    cl_float4 myPosition = globalP[global_id];
    int i;

    cl_float4 acc = {{0.0f, 0.0f, 0.0f, 1.0f}};
    
    for (i = 0; i < points; i ++) {
	bodyBodyInteraction(myPosition, globalP[i], &acc);
    }
    globalA[global_id] = acc;
}

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

cl_float4 * initializeAccelerations() {
    cl_float4 * pts = (cl_float4*) malloc(sizeof(cl_float4)*POINTS);
    int i;

    for (i = 0; i < POINTS; i++) {
	pts[i].x = pts[i].y = pts[i].z = pts[i].w = 0;
    }
    return pts;
}

std::string* loadKernel() {
  std::ifstream file("src/bruteforce-kernel.cl"); 
  std::string* contents = new std::string((std::istreambuf_iterator<char>(file)),
                                          (std::istreambuf_iterator<char>()));

  return contents;
}

int main(int argc, char ** argv)
{
    cl_float4 * x = initializePositions();
    cl_float4 * a = initializeAccelerations();

    int i;
    for (i = 0; i < POINTS; i++)
	calculateForces(POINTS, i, x, a);

    for (i = 0; i < POINTS; i++)
	printf("(%2.2f,%2.2f,%2.2f,%2.2f) (%2.3f,%2.3f,%2.3f)\n", 
	       x[i].x, x[i].y, x[i].z, x[i].w,
	       a[i].x, a[i].y, a[i].z);
    free(x);
    free(a);
    return 0;
}

cl_float4* runKernel(const char* source)
{
    cl_platform_id platform_id;
    clGetPlatformIDs(1, &platform_id, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "nbody", NULL);

    cl_mem position_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, POINTS * sizeof(cl_float4), NULL, NULL);
    cl_mem accelleration_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, POINTS * sizeof(cl_float4), NULL, NULL);

    int work_size = POINTS;
    size_t size = POINTS;
    clSetKernelArg(kernel, BUFFER_SIZE, sizeof(work_size), (void*) &work_size);
    clSetKernelArg(kernel, POSITION_BUFFER, sizeof(position_buffer), (void*) &position_buffer);
    clSetKernelArg(kernel, ACCELLERATION_BUFFER, sizeof(accelleration_buffer), (void*) &accelleration_buffer);

    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &size, NULL, 0, NULL, NULL);

    clFinish(queue);

    cl_float4* ptr;
    ptr = (cl_float4*) clEnqueueMapBuffer(queue, accelleration_buffer, CL_TRUE, CL_MAP_READ, 0, POINTS*sizeof(cl_float4), 0, NULL, NULL, NULL);

    return ptr;    
}
