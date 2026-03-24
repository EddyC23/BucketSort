#pragma once
#include <windows.h>
#include "StreamManager.h"
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
		StreamManager* sm;
		//pointer to the buckets
		uint64_t*** buckets;
		void sort(uint64_t* buffer, uint64_t size, int shift, int level);
	public:	
		BucketSort(StreamManager* sm, uint64_t * inputBuffer, uint64_t * outputBuffer, uint64_t size, int flag);
		void sort();
		void printArray(uint64_t* ptr, uint64_t size);
		bool isSorted();
		
};