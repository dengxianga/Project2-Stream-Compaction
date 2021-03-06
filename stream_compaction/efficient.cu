﻿#include <cuda.h>
#include <cuda_runtime.h>
#include "common.h"
#include "efficient.h"

namespace StreamCompaction {
namespace Efficient {

// TODO: __global__
__global__ void upSweep(int offset, int n,   int *idata){
	int index = threadIdx.x + blockIdx.x*blockDim.x;
	if (index >=n) return;
	int tmp=(offset << 1);
	if (index % tmp==0){
		if (index + tmp <=n){ 
			idata[index+tmp-1] += idata[index+offset-1]  ;		 
		}
	}
}

__global__ void downSweep(int offset, int n,  int *idata){
	int index = threadIdx.x + blockIdx.x*blockDim.x;
	if (index >=n) return;
	int tmp=(offset << 1);
	if (index % tmp==0){

		if (index + tmp <= n){
			int t = idata[index + offset -1];
			idata[index+offset-1] = idata[index+ tmp -1];
			idata[index+ tmp -1] += t ;
		}
 
	}
}


/**
 * Performs prefix-sum (aka scan) on idata, storing the result into odata.
 */
float scan(int n, int *odata, const int *idata) {
	cudaEvent_t t0, t2;
	cudaEventCreate(&t0);
	cudaEventCreate(&t2); 
 
	float milliscs = 0.0f;
	float tmpt;
    // TODO
    //printf("TODO\n");
	int levels_max = ilog2ceil(n);
	int n_max= 1 << levels_max;

	dim3 numblocks(std::ceil((double) n_max / blockSize));
	int* idata_buff;
	//allocate more space than needed
	cudaMalloc((void**)&idata_buff, n_max*sizeof(int)); 	
		checkCUDAError("cudaMalloc-idata_buff-  failed!");	
	//reset all to zeros
    cudaMemset(idata_buff, 0, n_max*sizeof(int));
		checkCUDAError("cudaMemset-idata_buff-  failed!");	

	/// CPU -->GPU
	cudaMemcpy(idata_buff,idata,n*sizeof(int),cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy-idata_buff-failed");

	cudaEventRecord(t0);

	//upsweep
	for (int level=0; level <= levels_max-1; level++){
		upSweep<<<numblocks,blockSize>>>(1<<level, n_max, idata_buff);
	}

	cudaEventRecord(t2);	
	cudaEventSynchronize(t2);
	cudaEventElapsedTime(&tmpt, t0, t2);	 
	milliscs += tmpt;


	//downsweep
	//set root x[n-1]=0
	//idata_buff[n_max-1]=0;
	cudaMemset(idata_buff+n_max-1, 0,  sizeof(int));
		
	cudaEventRecord(t0);

	for (int level=levels_max-1; level >=0 ; level--){
		downSweep<<<numblocks,blockSize>>>(1<<level, n_max, idata_buff);
	}

	cudaEventRecord(t2);
	cudaEventSynchronize(t2);	
	cudaEventElapsedTime(&tmpt, t0, t2);
	milliscs += tmpt;

	/// GPU --> CPU
	cudaMemcpy(odata, idata_buff, n*sizeof(int),cudaMemcpyDeviceToHost);
		checkCUDAError("cudaMemcpy-odata-failed");
	cudaFree(idata_buff);
	return milliscs;
}

/**
 * Performs stream compaction on idata, storing the result into odata.
 * All zeroes are discarded.
 *
 * @param n      The number of elements in idata.
 * @param odata  The array into which to store elements.
 * @param idata  The array of elements to compact.
 * @returns      The number of elements remaining after compaction.
 */
int compact(int n, int *odata, const int *idata, float &milliscs) {
	cudaEvent_t t0, t2;
	cudaEventCreate(&t0);
	cudaEventCreate(&t2);

	milliscs = 0.0f;
	float tmpt;


    int n_remaing=0;
	int * idata_buff;
	int * odata_buff;
	int * bool_buff;
	int * indices_buff;

	dim3 numblocks(std::ceil((double) n/blockSize));
	//
	cudaMalloc((void**)&idata_buff,n * sizeof(int));
		checkCUDAError("cudaMalloc-idata_buff-failed");
	cudaMalloc((void**)&odata_buff,n * sizeof(int));
		checkCUDAError("cudaMalloc-odata_buff-failed");
	cudaMalloc((void**)&bool_buff,n * sizeof(int));
		checkCUDAError("cudaMalloc-odata_buff-failed");
	cudaMalloc((void**)&indices_buff,n * sizeof(int));
		checkCUDAError("cudaMalloc-odata_buff-failed");

	cudaMemcpy(idata_buff, idata, n* sizeof(int), cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy-idata_buff-failed");
	cudaMemcpy(odata_buff, odata, n* sizeof(int), cudaMemcpyHostToDevice);
		checkCUDAError("cudaMemcpy-odata_buff-failed");
	
	cudaEventRecord(t0);
	//produce the indices
	StreamCompaction::Common::kernMapToBoolean<<<numblocks, blockSize>>> ( n, bool_buff, idata_buff);

	scan  (n, indices_buff, bool_buff);

	StreamCompaction::Common::kernScatter<<<numblocks, blockSize>>>( n, odata_buff, idata_buff,  bool_buff,  indices_buff);
	
	cudaEventRecord(t2);
	cudaEventSynchronize(t2);	
	cudaEventElapsedTime(&tmpt, t0, t2);
	milliscs += tmpt;

	//GPU-->CPU
	cudaMemcpy(odata,odata_buff,n*sizeof(int),cudaMemcpyDeviceToHost);

	//for (int i =0; i< n; i++){
	//	n_remaing+=bool_buff[i];
	//}
	cudaMemcpy(&n_remaing,indices_buff+n-1,sizeof(int),cudaMemcpyDeviceToHost);
	int extra;
	cudaMemcpy(&extra, bool_buff + n - 1, sizeof(int), cudaMemcpyDeviceToHost);
	cudaFree(idata_buff);
	cudaFree(odata_buff);
	cudaFree(bool_buff);
	cudaFree(indices_buff);
	return n_remaing + extra;
}

}
}
