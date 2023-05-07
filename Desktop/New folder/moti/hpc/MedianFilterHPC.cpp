//how to implement median filter in cuda?
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#define FILTER_WIDTH 3
#define ELEM_SWAP(a,b) {register unsigned char t=(a);(a)=(b);(b)=t;}

using namespace cv;
using namespace std;


__global__ void medianFilter(unsigned char* d_input, unsigned char* d_output, int rows, int cols){

	int idx = threadIdx.x + blockIdx.x*blockDim.x;
	int idy = threadIdx.y + blockIdx.y*blockDim.y;

	int mid = idy*cols + idx;

	int size = FILTER_WIDTH*FILTER_WIDTH;

	if (idx >= cols || idy >= rows)
		return;

	unsigned char value;

	unsigned char medianFilt[FILTER_WIDTH*FILTER_WIDTH];

	//CONVOLUTION 
	for (int i = -FILTER_WIDTH / 2; i <= FILTER_WIDTH / 2; i++){

		for (int j = -FILTER_WIDTH / 2; j <= FILTER_WIDTH / 2; j++){

			int x = idx + i;
			int y = idy + j;

			int filt_x = i + FILTER_WIDTH / 2;
			int filt_y = j + FILTER_WIDTH / 2;

			if (x=>0 && x<cols && y=>0 && y<rows){

				medianFilt[filt_y*FILTER_WIDTH + filt_x] = d_input[y*cols + x];

			}

		}

	}

	//QUICK_SELECT 
	int low, high;
	int median;
	int middle, ll, hh;

	low = 0; high = size - 1; median = (low + high) / 2;
	for (;;) {
		if (high <= low){ /* One element only */
			value = medianFilt[median];
			break;
		}
		if (high == low + 1) {  /* Two elements only */
			if (medianFilt[low] > medianFilt[high])
				ELEM_SWAP(medianFilt[low], medianFilt[high]);
			value = medianFilt[median];
			break;
		}

		middle = (low + high) / 2;
		if (medianFilt[middle] > medianFilt[high])    ELEM_SWAP(medianFilt[middle], medianFilt[high]);
		if (medianFilt[low] > medianFilt[high])       ELEM_SWAP(medianFilt[low], medianFilt[high]);
		if (medianFilt[middle] > medianFilt[low])     ELEM_SWAP(medianFilt[middle], medianFilt[low]);

		ELEM_SWAP(medianFilt[middle], medianFilt[low + 1]);

		ll = low + 1;
		hh = high;
		for (;;) {
			do ll++; while (medianFilt[low] > medianFilt[ll]);
			do hh--; while (medianFilt[hh]  > medianFilt[low]);

			if (hh < ll)
				break;

			ELEM_SWAP(medianFilt[ll], medianFilt[hh]);
		}

		ELEM_SWAP(medianFilt[low], medianFilt[hh]);

		if (hh <= median)
			low = ll;
		if (hh >= median)
			high = hh - 1;
	}

	d_output[mid] = value;
	
}


int main(){

	unsigned char *d_img, *d_out;

	Mat image;
	Mat img;

	img = imread("MFExample.jpg", CV_LOAD_IMAGE_COLOR);

	if (!img.data){

		printf("Couldn't open current image !!! ");
		return -1;
	}
	image = Mat::zeros(img.rows, img.cols, CV_8UC3);

	int Rows = img.rows;
	int Cols = img.cols;

	unsigned char* imgR = (unsigned char*)malloc(Rows*Cols*sizeof(unsigned char));
	unsigned char* imgG = (unsigned char*)malloc(Rows*Cols*sizeof(unsigned char));
	unsigned char* imgB = (unsigned char*)malloc(Rows*Cols*sizeof(unsigned char));

	for (int i = 0; i<Rows; i++){

		for (int j = 0; j<Cols; j++){
			//B-G-R
			imgB[i*Cols + j] = img.at<cv::Vec3b>(i, j)[0];
			imgG[i*Cols + j] = img.at<cv::Vec3b>(i, j)[1];
			imgR[i*Cols + j] = img.at<cv::Vec3b>(i, j)[2];

		}

	}
	imshow("Display Window",img);
	waitKey(0);


	unsigned char* h_out=(unsigned char*)malloc(Rows*Cols*sizeof(unsigned char));

	cudaMalloc((void**)&d_img, Rows*Cols*sizeof(unsigned char));
	cudaMemcpy(d_img, imgB, Rows*Cols*sizeof(unsigned char),cudaMemcpyHostToDevice);
	cudaMalloc((void**)&d_out, Rows*Cols*sizeof(unsigned char));

	dim3 blockSize(FILTER_WIDTH, FILTER_WIDTH, 1);
	dim3 gridSize((int)((Cols-1) / blockSize.x) + 1,(int)((Rows-1) / blockSize.y) + 1, 1);

	medianFilter << <gridSize, blockSize >> >(d_img, d_out, Rows, Cols);
	cudaMemcpy(h_out, d_out, Rows*Cols*sizeof(unsigned char),cudaMemcpyDeviceToHost);

	for (int i = 0; i < Rows;i++){
		for (int j = 0; j < Cols; j++){
			img.at<cv::Vec3b>(i, j)[0] = h_out[i*Cols + j];
		}
	}
	printf("**************\n\n\n");
	cudaMalloc((void**)&d_img,Rows*Cols*sizeof(unsigned char));
	cudaMemcpy(d_img, imgG, Rows*Cols*sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&d_out, Rows*Cols*sizeof(unsigned char));

	medianFilter << <gridSize, blockSize >> >(d_img, d_out, Rows, Cols);
	cudaMemcpy(h_out, d_out, Rows*Cols*sizeof(unsigned char), cudaMemcpyDeviceToHost);


	for (int i = 0; i < Rows; i++){
		for (int j = 0; j < Cols; j++){
			img.at<cv::Vec3b>(i, j)[1] = h_out[i*Cols + j];
		}
	}
	printf("**************\n\n\n");

	cudaMalloc((void**)&d_img,Rows*Cols*sizeof(unsigned char));
	cudaMemcpy(d_img, imgR, Rows*Cols*sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMalloc((void**)&d_out, Rows*Cols*sizeof(unsigned char));

	medianFilter << <gridSize, blockSize >> >(d_img, d_out, Rows, Cols);
	cudaMemcpy(h_out, d_out, Rows*Cols*sizeof(unsigned char), cudaMemcpyDeviceToHost);
	
	for (int i = 0; i < Rows; i++){
		for (int j = 0; j < Cols; j++){
			img.at<cv::Vec3b>(i, j)[2] = h_out[i*Cols + j];
		}
	}
	imshow("Display window", img);
	waitKey(0);

	cudaFree(d_img);
	cudaFree(d_out);


}