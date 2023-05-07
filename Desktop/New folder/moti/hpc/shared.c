

%%writefile MedianFilter.cu
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<cuda_runtime.h>

#define FILTER_SIZE 3
int cmpfunc (const void * a, const void * b) {
   return ( (int)a - (int)b );
}

/*
_global_ void median_filter(int *input, int *output, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int half_filter = FILTER_SIZE / 2;

        // Shared memory for storing the filter window
        _shared_ int window[FILTER_SIZE * FILTER_SIZE];

        // Fill the shared memory with the filter window
        for (int i = 0; i < FILTER_SIZE; i++) {
            for (int j = 0; j < FILTER_SIZE; j++) {
                int idx = (threadIdx.y + i) * blockDim.x + (threadIdx.x + j);
                if (threadIdx.x + j < blockDim.x && threadIdx.y + i < blockDim.y) {
                    window[i * FILTER_SIZE + j] = input[(y + i - half_filter) * width + (x + j - half_filter)];
                }
            }
        }

        // Wait for all threads in the block to finish filling the shared memory
        __syncthreads();

        // Perform the median operation on the filter window
        for (int i = 0; i < FILTER_SIZE * FILTER_SIZE; i++) {
            int min_idx = i;
            for (int j = i + 1; j < FILTER_SIZE * FILTER_SIZE; j++) {
                if (window[j] < window[min_idx]) {
                    min_idx = j;
                }
            }
            int tmp = window[i];
            window[i] = window[min_idx];
            window[min_idx] = tmp;
        }

        // Write the median value to the output array
        output[y * width + x] = window[FILTER_SIZE * FILTER_SIZE / 2];
    }
}*/

_global_ void median_filter(int *input, int *output, int rows, int cols)
{

    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    

    int windowSize = 3;

    int filter[9] {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };

    int pixelValues[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};


    if (
        (x != cols - windowSize +1  ||
        y != rows - windowSize +1 )
    )
    {
        return;
    }


    for (int hh = 0; hh < windowSize; hh++) 
    {
        for (int ww = 0; ww < windowSize; ww++) 
        {
            if (filter[hh * windowSize + ww] == 1)
            {
                int idx = (y + hh - 1) * cols + (x + ww - 1);
                pixelValues[hh * windowSize + ww] = input[idx];
            }
        }
    }

    int i, j;
    for (i = 0; i < 9 - 1; i++)
        for (j = 0; j < 9 - i - 1; j++)
            if (pixelValues[j] > pixelValues[j + 1]){
              int t = pixelValues[j];
              pixelValues[j] = pixelValues[j+1];
              pixelValues[j+1] = t;
            }

    int filteredValue = pixelValues[(windowSize * windowSize) / 2];
    output[y * cols + x] = filteredValue;
}



int main()
{
    int width = 3;
    int height = 3;

    // Allocate memory for the input and output arrays
    int input = (int) malloc(width * height * sizeof(int));
    int output = (int) malloc(width * height * sizeof(int));
    int *d_input, *d_output;
 
    cudaMalloc((void**)&d_input, width * height * sizeof(int));
    cudaMalloc((void**)&d_output, width * height * sizeof(int));

    // Initialize the input array with random values
    srand(time(NULL));
    for (int i = 0; i < width * height; i++) {
        input[i] = rand() % 256;
    }

    // Copy the input array to the GPU
    cudaMemcpy(d_input, input, width * height * sizeof(int), cudaMemcpyHostToDevice);

    // Set the block and grid dimensions
    int block_size = 3;
    dim3 dimGrid(ceil(width / (float)block_size), ceil(height / (float)block_size), 1);
    dim3 dimBlock(block_size, block_size, 1);

    // Launch the median filter kernel
    median_filter<<<dimGrid, dimBlock>>>(d_input, d_output, width, height);

    // Copy the output array back to the host
    cudaMemcpy(output, d_output, width * height * sizeof(int), cudaMemcpyDeviceToHost);

    // Print the first few values of the input and output arrays
    printf("Input:\n");
    for (int i = 0; i < width*height; i++) {
        printf("%d ", input[i]);
    }
    printf("\n");

    qsort(input, width*height, sizeof(int), cmpfunc);

    printf("Sorted Input:\n");
    for (int i = 0; i < width*height; i++) {
        printf("%d ", input[i]);
    }
    printf("\n");

    printf("Output:\n");
    for (int i = 0; i < width*height; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");

    // Free memory on the GPU and CPU
    cudaFree(d_input);
    cudaFree(d_output);
    free(input);
    free(output);

    return 0;
}


%%shell
nvcc MedianFilter.cu -o median
./median