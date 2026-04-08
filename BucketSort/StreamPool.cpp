#include "StreamPool.h"
#include "StreamManager.h"
#include <iostream>
StreamPool::StreamPool(uint64_t inputSizePower, uint64_t blockSizePower, uint64_t additionalBlocks) {
	this->inputSizePower = inputSizePower;
	this->blockSizePower = blockSizePower;
	this->additionalBlocks = additionalBlocks;
	this->numBlocks = (1ULL << (inputSizePower - blockSizePower)) * (1.0 + 1/255) + additionalBlocks;
	this->sizeArrayPFN = 1ULL << 20; //max capacity is 4 gb
	this->indexArrayPFN = 0;
	std::cout << "Total Blocks Allocated : " << numBlocks << "\n";
	std::cout << "Total Memory Allocated : " << ((1ULL << inputSizePower - blockSizePower) + additionalBlocks) / 1024.0 << "gb\n";
	uint64_t numPages = numBlocks << (blockSizePower - 12);

	//this->arrayPFN = new ULONG_PTR[sizeArrayPFN];
	this->arrayPFN = new ULONG_PTR[sizeArrayPFN];

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

	uint64_t pagesPerBlock = 1ULL << blockSizePower - 12;
	for (size_t i = 0; i < numBlocks; i++) {
		blockPool.push(arrayPFN + i * pagesPerBlock);
	}
	indexArrayPFN = numBlocks * pagesPerBlock;
}
void StreamPool::mapBlockFromPool(ULONG_PTR ptr) {
	StreamManager::blocksNeededCount = max(numBlocks - blockPool.size(), StreamManager::blocksNeededCount);
	void* vptr = (void*)ptr;

	if (blockPool.size() == 0) {
		requestAdditionalBlock();
	}
	PULONG_PTR pageArray = blockPool.top();
	uint64_t blockSizePages = 1ULL << (blockSizePower - 12);
	ptrToPFN[vptr] = pageArray;
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
	StreamManager::blocksNeededCount = max((indexArrayPFN / blockSizePages) - blockPool.size(), StreamManager::blocksNeededCount);
	blockPool.push(ptrToPFN[vptr]);
	if (!MapUserPhysicalPages(vptr, blockSizePages, NULL)) {
		std::cout << "unmap block failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
void StreamPool::requestAdditionalBlock() {
	uint64_t pagesPerBlock = 1ULL << (blockSizePower - 12); //pages per block
	blockPool.push(arrayPFN + indexArrayPFN);
	if (!AllocateUserPhysicalPages(GetCurrentProcess(),&pagesPerBlock, arrayPFN + indexArrayPFN)) {
		std::cout << "allocate user physical pages failed\n";
		std::cout << GetLastError();
		exit(-1);
	}
	if (pagesPerBlock != 1ULL << (blockSizePower - 12)) {
		std::cout << "allocate user physical pages allocated incorrect number of pages";
		exit(-1);
	}
	indexArrayPFN += pagesPerBlock;
}
uint64_t StreamPool::getSizeBlockPower() {
	return blockSizePower;
}
uint64_t StreamPool::getNumBlocks() {
	//num blocks alloc
	uint64_t blockSizePages = 1ULL << (blockSizePower - 12);
	return numBlocks + (indexArrayPFN) / blockSizePages;
}