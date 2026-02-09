#include "BucketSort.h"
BucketSort::BucketSort(uint64_t* inputBuffer, uint64_t* outputBuffer, uint64_t size) {
	this->inputBuffer = inputBuffer;
	this->outputBuffer = outputBuffer;
	this->size = size;
	this->depthRecursion = 8;
	this->numBuckets = 1 << 8;
	this->buckets = new uint64_t[8][1 << 8];

}
