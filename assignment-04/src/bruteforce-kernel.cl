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

__kernel void nbody(global const int points, global const cl_float4* P, global cl_float4* A) {
  int i;
  int id;
  cl_float4 myPosition;
  cl_float4 acc;

  id = get_global_id(0);
  myPosition = P[id];

  for (i = 0; i < points; i++) {
    bodyBodyInteraction(myPosition, P[i], &acc):
  }

  A[id] = acc;
};
