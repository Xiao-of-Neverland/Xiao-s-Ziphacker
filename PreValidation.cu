#include <cuda_runtime.h>
#include <cstdio>

__global__ void hello_cuda()
{
	printf("Hello cuda: %d\n", threadIdx.x);
}


void start_hello()
{
	hello_cuda<<<1, 4>>>();
}
