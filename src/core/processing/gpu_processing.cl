__kernel void processing(
       __global const float* a,
       ulong n1,
       __global float* d,
       ulong out,
       __global const float* b,
       ulong n2,
       __global const float* c,
       ulong n3
    )
{
    size_t i = get_global_id(0);
    if (i < n1) {
       d[i] = a[i] + b[i];
    }
}
