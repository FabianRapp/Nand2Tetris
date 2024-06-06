#include <cuda_runtime.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUFF_SIZE 100000

__global__ void remove_spaces_gpu(char *buffer, int read_bytes, int *byte_count)
{
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < read_bytes) {
        if (buffer[idx] != ' ') {
            int pos = atomicAdd(byte_count, 1);
            buffer[pos] = buffer[idx];
        }
    }
}

int main() {
    char hostBuffer[BUFF_SIZE];     // Original buffer on host
    char *devBuffer;      // Buffer on device (GPU)
    int read_bytes = 0; // Number of bytes to process
    int byte_count = 0;
    int *devByteCount;    // To store the result on device

	int fd = open("test_cuda.cu", O_RDONLY);
	if (errno)
		printf("%s\n", strerror(errno));
	errno = 0;
	read_bytes = read(fd ,hostBuffer,BUFF_SIZE);
	if (errno)
		printf("%s\n", strerror(errno));
	errno = 0;
	hostBuffer[read_bytes++] = 0;
	printf("read bytes: %d\n", read_bytes);
	printf("base: %s\n", hostBuffer);
    cudaMalloc((void **)&devBuffer, read_bytes);
    cudaMalloc((void **)&devByteCount, sizeof(int));

    cudaMemcpy(devBuffer, hostBuffer, read_bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(devByteCount, &byte_count, sizeof(int), cudaMemcpyHostToDevice);

    // Kernel launch parameters: Adjust these values based on your needs
    int threadsPerBlock = 256;
    int blocks = (read_bytes + threadsPerBlock - 1) / threadsPerBlock;

    // Launch the kernel
    remove_spaces_gpu<<<blocks, threadsPerBlock>>>(devBuffer, read_bytes, devByteCount);

    // Copy back the results
    cudaMemcpy(hostBuffer, devBuffer, read_bytes, cudaMemcpyDeviceToHost);
    cudaMemcpy(&byte_count, devByteCount, sizeof(int), cudaMemcpyDeviceToHost);

    // Cleanup
    cudaFree(devBuffer);
    cudaFree(devByteCount);
	printf("================================================================================================================\n");
	printf("count after: %d\n", byte_count);
	printf("result: %s\n", hostBuffer);
    return 0;
}
