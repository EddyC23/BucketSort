#pragma once
#include <windows.h>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <set>
class StreamManager;
class StreamPool {
public:
	StreamPool(uint64_t numBlocks, uint64_t blockSizePower, StreamManager* sm);
	void mapBlockFromPool(ULONG_PTR ptr);
	void unmapBlockToPool(ULONG_PTR ptr);
	void unmapBlockToPoolIterator(ULONG_PTR ptr, std::set<ULONG_PTR>::iterator& it, std::set<ULONG_PTR>& mapped);
	void requestAdditionalBlock();
	uint64_t getSizeBlockPower();
	void cleanUpBlocks(int i);
private:
	std::stack<PULONG_PTR> blockPool;
	std::unordered_map <void*, PULONG_PTR> ptrToPFN;
	std::unordered_map <int, std::set <ULONG_PTR> *> streamToMappedAddress;
	PULONG_PTR arrayPFN;
	uint64_t sizeArrayPFN;
	uint64_t indexArrayPFN;
	uint64_t blockSizePower; 
	StreamManager* sm;
};