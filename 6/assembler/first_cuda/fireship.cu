#include <stdio.h>

/*
Terms:
Block: A group of threads
Cuda kernel: code that runs on the gpu

Syntax:
__global__ a function type that indicates the function is a cuda kernel
threadIdx: Varibale avialble in cuda kernels. Each thread has its index
blockDim: Variable avialable in cuda kernels.
	
*/
 
// __global__ indicates that this function is run on gpu
__global__ void add(int *a, int *b, int *c)
{
	//threadIdx = buildin varaible that changes on runtime
	//is the threads id that is executing the job (so many of the same
	// threadIdx exist at the same time)
	//blockDim = 
	int i = threadIdx.x + blockDim.x;
	c[i] = a[i] + b[i];
}

// __mangaged__ vars can be accessed by cpu and gpu
__managed__ int vector_a[256], vector_b[256], vector_c[256];


void	test(void)
{
	for (int i = 0; i < 256; i++)
	{
		vector_a[i] = i;
		vector_b[i] = 256 - i;
	}
	//<< >>: configures the 'block' count and the threads per block
	add<<<1, 256>>>(vector_a, vector_b, vector_c);
	cudaDeviceSynchronize();
}

int main(void)
{

	return (0);
}

