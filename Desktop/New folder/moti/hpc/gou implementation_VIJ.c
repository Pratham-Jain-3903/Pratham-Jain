#include <stdio.h>
#include <stdlib.h>

#define FILTER_SIZE 3

int median(int arr[]) {
  int temp;

  // Sort the array
  for (int i = 0; i < FILTER_SIZE*FILTER_SIZE; i++) {
    for (int j = i + 1; j < FILTER_SIZE*FILTER_SIZE; j++) {
      if (arr[j] < arr[i]) {
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }

  // Return the median value
  return arr[(FILTER_SIZE*FILTER_SIZE) / 2];
}

int main() {
    int s = 8;
  int image[s][s];
  int filtered_image[s][s];
  
  // Populate the image with random values
  printf("Input image:\n");
  for (int i = 0; i < s; i++) {
    for (int j = 0; j < s; j++) {
      image[i][j] = i+j;
      filtered_image[i][j] = 0;
      printf("%d ", image[i][j]);
    }
    printf("\n");
  }
  
  // Apply the median filter to the image
  for (int i = 1; i < s-1; i++) {
    for (int j = 1; j < s-1; j++) {
      int neighbors[FILTER_SIZE * FILTER_SIZE];
      int index = 0;
      for (int k = i - 1; k <= i + 1; k++) {
        for (int l = j - 1; l <= j + 1; l++) {
          if (k >= 0 && k < s && l >= 0 && l < s) {
            neighbors[index] = image[k][l];
            index++;
          }
        }
      }
      
      filtered_image[i][j] = median(neighbors);
    }
  }
  
  // Print the filtered image
  printf("\nFiltered image:\n");
  for (int i = 0; i < s; i++) {
    for (int j = 0; j < s; j++) {
      printf("%d ", filtered_image[i][j]);
    }
    printf("\n");
  }
  
  return 0;
}