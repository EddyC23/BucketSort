#include "BucketSort.h"
#include "../Headers/StreamManager.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
uint64_t numWrites = 0;
void BucketSort::printArray(uint64_t* ptr, uint64_t size) {
	for (size_t i = 0; i < size; i++) {
		printf("index %lld : %llx %llu\n", i, ptr[i], ptr[i]);
	}
}

BucketSort::BucketSort(StreamManager* sm,uint64_t* inputBuffer, uint64_t* outputBuffer, uint64_t size, int flag) {
	this->sm = sm;
	this->inputBuffer = inputBuffer;
	this->outputBuffer = outputBuffer;
	this->outputBufferNext = outputBuffer;
	this->size = size;
	this->depthRecursion = 8;
	this->numBuckets = 256;
	this->buckets = new uint64_t **[8 + 1];
	this->flag = flag;
	for (size_t i = 0; i < depthRecursion + 1; i++) {
		this->buckets[i] = new uint64_t *[numBuckets];
	}
	for (size_t i = 0; i < numBuckets; i++) {
		this->buckets[0][i] = (uint64_t*)sm->getBucketStream(i)->getStartPtr();
	}
}
void BucketSort::sort() {
	outputBufferNext = outputBuffer;
	sort(inputBuffer, size, 56, 0);
}

void mymemcpy(uint64_t* source, uint64_t* destination, uint64_t length) {
	for (uint64_t i = 0; i < length; i++) {
		destination[i] = source[i];
	}

} // my memcpy

__declspec(noinline) void BucketSort::sort(uint64_t* buf, uint64_t size, int shift, int level) {
	
	
	//std::cout << level;
	uint64_t** p = buckets[level];
	uint64_t** pNext = buckets[level + 1]; // the buckets are not contiguous in virtual memory eg 0 is not immediately followed by 1
	memcpy(pNext, p, sizeof(uint64_t*) * numBuckets);

	
	for (uint64_t i = 0; i < size; i++) {
		uint64_t mask = (1 << 8) - 1;
		uint64_t idx = (buf[i] >> shift) & mask;
		*pNext[idx]++ = buf[i]; // write the numbner to the bucket at the current location, increment ptr
	}
	
	for (uint64_t j = 0; j < numBuckets; j++) {
		uint64_t sizeNext = pNext[j] - p[j];
		if (shift == 0) {
			//std::cout << "starting memcpy" << std::endl;
			//std::cout << "about to memcpy";
			mymemcpy(p[j], outputBufferNext, sizeNext);
			//memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			// std::cout << "done memcpy" << std::endl;
			outputBufferNext += sizeNext;
			numWrites += sizeNext;
		}else if (sizeNext <= 32) {
			std::sort(p[j], pNext[j]); // problem without fixing shift?
			//std::cout << "starting memcpy" << std::endl;
			memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			//std::cout << "done memcpy" << std::endl;
			outputBufferNext += sizeNext;
			numWrites += sizeNext;
		}
		else {
			sort(p[j], sizeNext, shift - 8, level + 1);
		}
		
		if (level == 0) {
			pNext[j] = (uint64_t*)sm->getBucketStream(j)->getStartPtr();
			for (size_t k = 0; k <= j; k++) {
				sm->cleanUpBlocks(k);
				sm->mapBlockFromPool(sm->getBucketStream(k)->getStartPtr());
			}
		}
		
	}
	
}
//check the max amount of block used
bool BucketSort::isSorted() {
	for (uint64_t i = 1; i < size; i++) {
		if (outputBuffer[i] < outputBuffer[i - 1]) {
			std::cout << "Not sorted.\n";
			return false;
		}
	}
	std::cout << "Sorted.\n";
	return true;
}

