#define EPS 1e-10

void bodyBodyInteraction(float4 bi, float4 bj, float4 *ai) {
    float4 r;
    
    r.x = bj.x - bi.x;
    r.y = bj.y - bi.y;
    r.z = bj.z - bi.z;
    r.w = 1.0f;

    float distSqr = r.x * r.x + r.y * r.y + r.z * r.z + EPS;

    float distSixth = distSqr * distSqr * distSqr;
    float invDistCube = 1.0f/sqrt(distSixth);

    float s = bj.w * invDistCube;

    ai->x += r.x * s;
    ai->y += r.y * s;
    ai->z += r.z * s;
}

__kernel void nbody(global const float4* P, global float4* A) {
  int i;
  int id;
  int size;
  float4 myPosition;
  float4 acc;

  id = get_global_id(0);
  size = get_global_size(0);

  myPosition = P[id];

  for (i = 0; i < size; i++) {
    bodyBodyInteraction(myPosition, P[i], &acc);
  }

  A[id] = acc;
};
