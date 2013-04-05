#define NUM_BINS 10
#define BIN_SIZE 100
#define POINTS 500*64

__kernel void bin(global const float4* points, global float4* cm) {
    int i;
    int id;
    float3 bin;
    float4 point;
    float4 centerMass;
    
    id = get_global_id(0);
    bin = (float3)(id % NUM_BINS * BIN_SIZE,
                   (id / NUM_BINS) % NUM_BINS * BIN_SIZE,
                   (id / (NUM_BINS*NUM_BINS)) % NUM_BINS * BIN_SIZE);


    centerMass = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < POINTS; i++) {
        point = points[i];

        if ((isgreaterequal(point.x, bin.x) && isless(point.x, (bin.x + BIN_SIZE))) && 
            (isgreaterequal(point.y, bin.y) && isless(point.y, (bin.y + BIN_SIZE))) && 
            (isgreaterequal(point.z, bin.z) && isless(point.z, (bin.z + BIN_SIZE)))) {

            // The point is within this bin

            centerMass.x += point.x;
            centerMass.y += point.y;
            centerMass.z += point.z;
            centerMass.w += 1.0f;         
        }
    }

    centerMass.x /= centerMass.w;
    centerMass.y /= centerMass.w;
    centerMass.z /= centerMass.w;
    cm[id] = centerMass;
};
