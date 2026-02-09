#pragma once
#include <windows.h>
#include <cstdint>
class BucketSort {
	private:
		int depthRecursion; 
		int numBuckets;
		uint64_t* inputBuffer;
		uint64_t* outputBuffer;
		uint64_t size;
		uint64_t* buckets[][] = new uint64_t[8][1 << 8];
		void sort(uint64_t* buffer, uint64_t size, int shift, int level);
	public:	
		BucketSort(uint64_t * inputBuffer, uint64_t * outputBuffer, uint64_t size);
};