// This is a naive implementation of a prefix sum
// algorithm
__kernel void prefix_sum(__global int* restrict A, __global int* restrict B, __private int numOfIterations)
{
    size_t tid = get_global_id(0);

    for (int d=0; d < numOfIterations; ++d)
    {
        if ( tid >= ( 1 << d) )
        {
            B[tid] = A[ tid - (1 << d)] + A[tid];
        }
        else
        {
            // Copy over the unmodifed element of array
            // We need these values to be propagated across
            // because we switch the roles of A and B every
            // loop iteration.
            B[tid] = A[tid];
        }
        barrier(CLK_GLOBAL_MEM_FENCE);

        // swap pointers for next iteration
        __global int* temp = A;
        A = B;
        B = temp;
    }

    // The final result is in B if # of loop iterations is odd.
    // The final result is in A if # of loop iterations is even.
}
