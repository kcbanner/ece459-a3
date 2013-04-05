#define EPS 1e-10
#define BINS 1000
#define NUM_BINS 10
#define BIN_SIZE 100

void bodyBodyInteraction(float4 bi, float4 bj, float4 *ai) {
    float4 r;
    
    r.x = bj.x - bi.x;
    r.y = bj.y - bi.y;
    r.z = bj.z - bi.z;
    r.w = 1.0f;

    float distSqr = r.x * r.x + r.y * r.y + r.z * r.z + EPS;

    float distSixth = distSqr * distSqr * distSqr;
    float invDistCube = rsqrt(distSixth);

    float s = bj.w * invDistCube;

    ai->x += r.x * s;
    ai->y += r.y * s;
    ai->z += r.z * s;
}

// Iterating once per bin

__kernel void forces(global const float4* points,
                     global const float4* cm,
                     global const unsigned int* offsets,
                     global const unsigned int* bins,
                     global float4* out) {
    int i;
    int id;
    int size;
    float4 myPosition;
    float4 acc;

    id = get_global_id(0);
    size = get_global_size(0);

    // First, accumulate all the forces from the other approximated bins' centers of mass

    myPosition = points[id];
    acc = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    for (i = 0; i < BINS; i++) {
        bodyBodyInteraction(myPosition, cm[i], &acc);
    }
    
    // Next, find out what bin we are in

    int3 bin;
    bin.x = myPosition.x / BIN_SIZE;
    bin.y = myPosition.y / BIN_SIZE;
    bin.z = myPosition.z / BIN_SIZE;

    int binId = 
        bin.x + 
        bin.y * NUM_BINS + 
        bin.z * NUM_BINS * NUM_BINS;
    
    // Do the bodyBodyInteraction for every point in all the 3x3 bins surrounding us

    out[id] = acc;
};
