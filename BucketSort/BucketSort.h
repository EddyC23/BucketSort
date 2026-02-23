#pragma once
#include <windows.h>
#include <cstdint>
class BucketSort {
	private:
		int depthRecursion; 
		int numBuckets;
		int flag; // flag to stop after this level
		uint64_t* inputBuffer;
		uint64_t* outputBuffer;
		uint64_t* outputBufferNext;
		uint64_t size;
		//pointer to the buckets
		uint64_t*** buckets;
		void sort(uint64_t* buffer, uint64_t size, int shift, int level);
	public:	
		BucketSort(uint64_t * inputBuffer, uint64_t * outputBuffer, uint64_t size, int flag);
		void sort();
		void printArray(uint64_t* ptr, uint64_t size);
		bool isSorted();
};