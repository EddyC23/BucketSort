#include "BucketSort.h"
#include <algorithm>
BucketSort::BucketSort(uint64_t* inputBuffer, uint64_t* outputBuffer, uint64_t size) {
	this->inputBuffer = inputBuffer;
	this->outputBuffer = outputBuffer;
	this->outputBufferNext = outputBuffer;
	this->size = size;
	this->depthRecursion = 8;
	this->numBuckets = 1 << 8;
	this->buckets = new uint64_t **[8 + 1];
	//bucket is a triple pointer
	// pointer to a array(pointer) that is (array of int * [numBckets])
	for (size_t i = 0; i < depthRecursion + 1; i++) {
		this->buckets[i] = new uint64_t *[numBuckets];
	}
	for (size_t i = 0; i < numBuckets; i++) {
		this->buckets[0][i] = new uint64_t[(1 << 16) * 8];
	}

	//D + 1 because need one more row to store the size of the buckets 
}
void BucketSort::sort() {
	outputBufferNext = outputBuffer;
	sort(inputBuffer, size, 56, 0);
}
void BucketSort::sort(uint64_t* buf, uint64_t size, int shift, int level) {
	

	uint64_t** p = &buckets[level][0];
	uint64_t** pNext = p + numBuckets;
	memcpy(pNext, p, sizeof(uint64_t*) * numBuckets);

	for (uint64_t i = 0; i < size; i++) {
		uint64_t mask = (1 << 8) - 1;
		uint64_t idx = (buf[i] >> shift) & mask;
		*pNext[idx]++ = buf[i];
	}

	for (uint64_t j = 0; j < numBuckets; j++) {
		uint64_t sizeNext = pNext[j] - p[j];
		//base case
		if (shift == 0) {
			memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			outputBufferNext += sizeNext;
		}else if (sizeNext > 32) {
			sort(p[j], sizeNext, shift - 8, level + 1);
		}
		else {
			std::sort(p[j], p[j] + sizeNext);
			memcpy(outputBufferNext, p[j], sizeof(uint64_t) * sizeNext);
			outputBufferNext += sizeNext;
		}

	}
}