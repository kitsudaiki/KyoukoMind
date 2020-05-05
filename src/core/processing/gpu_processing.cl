__kernel void add(
       __global const float* a,
       ulong n1,
       __global const float* b,
       ulong n2,
       __global float* c,
       ulong out
       )
{
    size_t i = get_global_id(0);
    if (i < n1) {
       c[i] = a[i] + b[i];
    }
}
