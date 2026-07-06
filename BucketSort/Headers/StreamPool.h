#pragma once
#include <windows.h>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <set>
#include <vector>
class StreamManager;
class StreamPool {
public:
	StreamPool(uint64_t numBlocks, uint64_t blockSizePower, StreamManager* sm);
	~StreamPool();
	void mapBlockFromPool(ULONG_PTR ptr);
	void unmapBlockToPool(ULONG_PTR ptr);
	void requestAdditionalBlock();
	uint64_t getSizeBlockPower();
	void cleanUpBlocks(int streamIndex);
	void getBlocksLeftBehindThread(int streamIndex, std::vector<int>& blocksLeft);
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