#include "StreamPool.h"
#include <iostream>
StreamPool::StreamPool(uint64_t inputSizePower, uint64_t blockSizePower, uint64_t additionalBlocks) {
	this->inputSizePower = inputSizePower;
	this->blockSizePower = blockSizePower;
	this->additionalBlocks = additionalBlocks;

	uint64_t numBlocks = (1ULL << (inputSizePower - blockSizePower)) + additionalBlocks;
	uint64_t numPages = numBlocks << blockSizePower >> 12;
	this->arrayPFN = new ULONG_PTR[numPages];

	if (blockSizePower < 12) {
		std::cout << "block size power has to be at least 12";
		exit(-1);
	}
	if (!AllocateUserPhysicalPages(GetCurrentProcess(), &numPages, arrayPFN)) {
		std::cout << "allocate user physical pages failed\n";
		std::cout << GetLastError();
		exit(-1);
	}
	if (numPages != numBlocks << blockSizePower >> 12) {
		std::cout << "allocate user physical pages allocated incorrect number of pages";
		exit(-1);
	}

	uint64_t pagesPerBlock = 1ULL << blockSizePower >> 12;
	for (size_t i = 0; i < numBlocks; i++) {
		blockPool.push(arrayPFN + i * pagesPerBlock);
	}
}
void StreamPool::mapBlockFromPool(ULONG_PTR ptr) {
	void* vptr = (void*)ptr;
	PULONG_PTR pageArray = blockPool.top();
	uint64_t blockSizePages = 1ULL << (blockSizePower - 12);
	blockPool.pop();
	if (!MapUserPhysicalPages(vptr, blockSizePages, pageArray)) {
		std::cout << "map block failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
void StreamPool::unmapBlockToPool(ULONG_PTR ptr) {
	void* vptr = (void*)ptr;
	uint64_t blockSizePages = 1ULL << (blockSizePower - 12);
	blockPool.push(ptrToPFN[vptr]);
	if (!MapUserPhysicalPages(vptr, blockSizePages, NULL)) {
		std::cout << "unmap block failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
uint64_t StreamPool::getSizeBlockPower() {
	return blockSizePower;
}