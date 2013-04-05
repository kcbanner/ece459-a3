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
    bin.x = trunc(myPosition.x / (float)BIN_SIZE);
    bin.y = trunc(myPosition.y / (float)BIN_SIZE);
    bin.z = trunc(myPosition.z / (float)BIN_SIZE);
    
    // Do the bodyBodyInteraction for every point in all the 3x3 bins surrounding us

    int2 x = (int2)(bin.x - 1, bin.x + 1);
    int2 y = (int2)(bin.y - 1, bin.y + 1);
    int2 z = (int2)(bin.z - 1, bin.z + 1);
    x = clamp(x, 0, 9);
    y = clamp(y, 0, 9);
    z = clamp(z, 0, 9);
    
    int j;
    int k;
    for (i = x.x; i <= x.y; i++) {
        for (j = y.x; j <= y.y; j++) {
            for (k = z.x; k <= z.y; k++) {
                
                // Loop over everything in this bin and bodyBodyInteraction it

                int binId = 
                    i + 
                    j * NUM_BINS + 
                    k * NUM_BINS * NUM_BINS;

                int binIter;
                for (binIter = 0; binIter < (int) cm[binId].w; binIter++) {
                    bodyBodyInteraction(myPosition,
                                        points[bins[offsets[binId] + binIter]],
                                        &acc);
                }
                
                float4 negBin;
                negBin.x = 2*myPosition.x-cm[binId].x;
                negBin.y = 2*myPosition.y-cm[binId].y;
                negBin.z = 2*myPosition.z-cm[binId].z;
                negBin.w = cm[binId].w;
                bodyBodyInteraction(myPosition, negBin, &acc);

            }
        }
    }

    out[id] = acc;
};
