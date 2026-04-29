#pragma once
#include <windows.h>
#include <cstdint>
#include "StreamPool.h"
#include <vector>
class VortexS {
public:
	VortexS(uint64_t sizeStreamPower, StreamPool* blockPool);
	LONG handle_exception(PEXCEPTION_POINTERS info);
	ULONG_PTR getStartPtr();
	ULONG_PTR getEndPtr();
	std::vector<int> blocksLeftBehind();
	void blocksLeftBehindThread(std::vector<int>& blocks);


private:
	void* startPtr;
	void* endPtr;
	uint64_t sizeStreamPower;
	uint64_t sizeBlockPower;
	ULONG_PTR lastReadFault;
	bool isLastReadFaultValid;
	StreamPool* blockPool;
	DWORD setGuardPage(ULONG_PTR ptr);
	DWORD removeGuardPage(ULONG_PTR ptr);
};