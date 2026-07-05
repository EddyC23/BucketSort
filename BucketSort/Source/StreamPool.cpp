#include "StreamPool.h"
#include "StreamManager.h"
#include <iostream>
StreamPool::StreamPool(uint64_t numBlocks, uint64_t blockSizePower, StreamManager *sm) {
	this->blockSizePower = blockSizePower;
	this->sizeArrayPFN = 1ULL << 20; //max capacity is 4 gb
	this->indexArrayPFN = 0;
	this->sm = sm;
	uint64_t numPages = numBlocks << (blockSizePower - 12);
	this->arrayPFN = new ULONG_PTR[sizeArrayPFN];
	for (int i = 0; i < 258; i++) {
		this->streamToMappedAddress[i] = new std::set<ULONG_PTR>;
	}
	if (blockSizePower < 12) {
		std::cout << "block size power has to be at least 12";
		exit(-1);
	}
	if (!AllocateUserPhysicalPages(GetCurrentProcess(), &numPages, arrayPFN)) {
		std::cout << "allocate user physical pages failed\n";
		std::cout << GetLastError();
		exit(-1);
	}
	if (numPages != numBlocks << (blockSizePower - 12)) {
		std::cout << "allocate user physical pages allocated incorrect number of pages";
		exit(-1);
	}
	uint64_t pagesPerBlock = 1ULL << (blockSizePower - 12);
	for (size_t i = 0; i < numBlocks; i++) {
		blockPool.push(arrayPFN + i * pagesPerBlock);
	}
	indexArrayPFN = numBlocks * pagesPerBlock;
}
void StreamPool::mapBlockFromPool(ULONG_PTR ptr) {
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
	StreamManager::mapCount++;
	StreamManager::blocksNeededCount = max(StreamManager::mapCount - StreamManager::unmapCount, StreamManager::blocksNeededCount);

	this->streamToMappedAddress[sm->getStreamIndexFromAddressLinear(ptr)]->insert(ptr);

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
	StreamManager::unmapCount++;
	this->streamToMappedAddress[sm->getStreamIndexFromAddressLinear(ptr)]->erase(ptr);

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
	StreamManager::requestedBlocks++;
}
uint64_t StreamPool::getSizeBlockPower() {
	return blockSizePower;
}
void StreamPool::cleanUpBlocks(int streamIndex) {
	std::set<ULONG_PTR>* mappedAddress = this->streamToMappedAddress[streamIndex + 2];
	for (auto it = mappedAddress->begin(); it != mappedAddress->end();) {
		//if (*it == sm->getNthStream(streamIndex)->getStartPtr()) {
		//	it++;
		//	continue;
		//}
		void* vptr = (void*)*it;
		uint64_t blockSizePages = 1ULL << (blockSizePower - 12);
		if (!MapUserPhysicalPages(vptr, blockSizePages, NULL)) {
			std::cout << "unmap block failed";
			std::cout << GetLastError();
			exit(-1);
		}
		blockPool.push(ptrToPFN.find(vptr)->second);
		StreamManager::unmapCount++;
		it = mappedAddress->erase(it);
	}
}

void StreamPool::getBlocksLeftBehindThread(int streamIndex, std::vector<int>& blocksLeft) {
	std::set<ULONG_PTR>* mappedAddress = this->streamToMappedAddress[streamIndex + 2];
	ULONG_PTR startPtr = sm->getBucketStream(streamIndex)->getStartPtr();
	for (auto it = mappedAddress->begin(); it != mappedAddress->end(); it++) {
		ULONG_PTR currPtr = *it;
		int blockIndex = (startPtr - currPtr) >> blockSizePower;
		blocksLeft.push_back(blockIndex);
	}
}