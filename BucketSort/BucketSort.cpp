#include "BucketSort.h"
#include "StreamManager.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

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
	this->numBuckets = 1 << 8;
	this->buckets = new uint64_t **[8 + 1];
	this->flag = flag;
	
	for (size_t i = 0; i < depthRecursion + 1; i++) {
		this->buckets[i] = new uint64_t *[numBuckets];
	}
	for (size_t i = 0; i < numBuckets; i++) {
		this->buckets[0][i] = (uint64_t*)sm->getNthStream(i)->getStartPtr();
	}
}
void BucketSort::sort() {
	outputBufferNext = outputBuffer;
	sort(inputBuffer, size, 56, 0);
}

void BucketSort::sort(uint64_t* buf, uint64_t size, int shift, int level) {
	uint64_t** p = buckets[level];
	uint64_t** pNext = buckets[level + 1]; // the buckets are not contiguous in virtual memory eg 0 is not immediately followed by 1
	memcpy(pNext, p, sizeof(uint64_t*) * numBuckets);

	for (uint64_t i = 0; i < size; i++) {
		uint64_t mask = (1 << 8) - 1;
		uint64_t idx = (buf[i] >> shift) & mask;
		*pNext[idx]++ = buf[i]; // write the numbner to the bucket at the current location, increment ptr
	}
	if (flag == level) {
		return;
	}
	

	for (uint64_t j = 0; j < numBuckets; j++) {
		uint64_t sizeNext = pNext[j] - p[j];
		if (shift == 0) {
			memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			outputBufferNext += sizeNext;
		}else if (sizeNext <= 32) {
			std::sort(p[j], pNext[j]);
			memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			outputBufferNext += sizeNext;
		}
		else {
			sort(p[j], sizeNext, shift - 8, level + 1);
		}

	}
	
}

bool BucketSort::isSorted() {
	for (uint64_t i = 1; i < size; i++) {
		if (outputBuffer[i] < outputBuffer[i - 1]) {
			std::cout << "Not sorted";
			return false;
		}
	}
	sm->printDebug();
	std::cout << "Sorted";
	return true;
}

