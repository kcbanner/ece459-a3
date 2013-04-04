#define BIN_SIZE 100

__kernel void bin(global const float4* points, global float4* cm) {
    int i;
    int id;
    int size;
    float3 bin;
    float4 point;
    
    id = get_global_id(0);
    size = get_global_size(0);

    bin = (float3)(id % BIN_SIZE,
                   (id / BIN_SIZE) % BIN_SIZE,
                   (id / (BIN_SIZE*BIN_SIZE)) % BIN_SIZE);

    for (i = 0; i < size; i++) {
        point = points[i];

        if ((point.x >= bin.x && point.x <= (bin.x + BIN_SIZE)) && 
            (point.y >= bin.y && point.y <= (bin.y + BIN_SIZE)) && 
            (point.z >= bin.z && point.z <= (bin.z + BIN_SIZE))) {

            cm[id].x += point.x;
            cm[id].y += point.y;
            cm[id].z += point.z;
            cm[id].w += 1.0f;                
        }
    }

    cm[id].x /= cm[id].w;
    cm[id].y /= cm[id].w;
    cm[id].z /= cm[id].w;
};
