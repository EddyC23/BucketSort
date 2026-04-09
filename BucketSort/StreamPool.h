#pragma once
#include <windows.h>
#include <cstdint>
#include <stack>
#include <unordered_map>
class StreamPool {
public:
	StreamPool(uint64_t numBlocks, uint64_t blockSizePower);
	void mapBlockFromPool(ULONG_PTR ptr);
	void unmapBlockToPool(ULONG_PTR ptr);
	void requestAdditionalBlock();
	uint64_t getSizeBlockPower();
private:
	std::stack<PULONG_PTR> blockPool;
	std::unordered_map <void*, PULONG_PTR> ptrToPFN;
	PULONG_PTR arrayPFN;
	uint64_t sizeArrayPFN;
	uint64_t indexArrayPFN;
	uint64_t blockSizePower; 
};