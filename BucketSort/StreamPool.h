#pragma once
#include <windows.h>
#include <cstdint>
#include <stack>
#include <unordered_map>
class StreamPool {
public:
	StreamPool(uint64_t inputSizePower, uint64_t blockSizePower, uint64_t additionalBlocks);
	void mapBlockFromPool(ULONG_PTR ptr);
	void unmapBlockToPool(ULONG_PTR ptr);
	void requestAdditionalBlock();
	uint64_t getSizeBlockPower();
	uint64_t getNumBlocks();
private:
	uint64_t inputSizePower;
	uint64_t additionalBlocks;
	std::stack<PULONG_PTR> blockPool;
	std::unordered_map <void*, PULONG_PTR> ptrToPFN;
	PULONG_PTR arrayPFN;
	uint64_t sizeArrayPFN;
	uint64_t indexArrayPFN;
	uint64_t blockSizePower; 
	uint64_t numBlocks;
};