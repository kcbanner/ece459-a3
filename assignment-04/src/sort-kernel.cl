#define BIN_SIZE 100

__kernel void sort(global const float4* points, global const int* offsets, global int* bins) {
    int i;
    int j;
    int id;
    int size;
    float3 bin;
    float4 point;
    
    id = get_global_id(0);
    size = get_global_size(0);

    bin = (float3)(id % BIN_SIZE,
                   (id / BIN_SIZE) % BIN_SIZE,
                   (id / (BIN_SIZE*BIN_SIZE)) % BIN_SIZE);

    int j = 0;
    for (i = 0; i < size; i++) {
        point = points[i];

        if ((point.x >= bin.x && point.x < (bin.x + BIN_SIZE)) && 
            (point.y >= bin.y && point.y < (bin.y + BIN_SIZE)) && 
            (point.z >= bin.z && point.z < (bin.z + BIN_SIZE))) {

            // The point is within this bin
            
            bins[offsets[id] + j] = i;
            j++;

        }
    }
};
