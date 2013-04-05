#define NUM_BINS 10
#define BIN_SIZE 100
#define POINTS 500*64

__kernel void sort(global const float4* points,
                   global const unsigned int* offsets,
                   global unsigned int* bins) {
    int i;
    int j;
    int id;
    int offset;
    float3 bin;
    float4 point;
    
    id = get_global_id(0);
    bin = (float3)(id % NUM_BINS * BIN_SIZE,
                   (id / NUM_BINS) % NUM_BINS * BIN_SIZE,
                   (id / (NUM_BINS*NUM_BINS)) % NUM_BINS * BIN_SIZE);

    j = 0;
    offset = offsets[id];
    for (i = 0; i < POINTS; i++) {
        point = points[i];

        if ((point.x >= bin.x && point.x < (bin.x + BIN_SIZE)) && 
            (point.y >= bin.y && point.y < (bin.y + BIN_SIZE)) && 
            (point.z >= bin.z && point.z < (bin.z + BIN_SIZE))) {

            // The point is within this bin
            
            bins[offset + j] = i;
            j++;

        }
    }
};
