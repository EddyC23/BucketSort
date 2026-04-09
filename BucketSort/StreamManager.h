#pragma once
#include "VortexS.h"
#include "StreamPool.h"
#include <unordered_map>
#include <set>
class StreamManager {
public:
	StreamManager(uint64_t numStreams, uint64_t sizeStreamPower, uint64_t sizeBlockPower, uint64_t additionalBlocks);
	VortexS* getStreamFromAddressLinear(ULONG_PTR faultAddress);
	VortexS* getInputStream();
	VortexS* getOutputStream();
	VortexS* getNthStream(int n);
	static int guardCount;
	static int mapCount;
	static int unmapCount;
	static int blocksNeededCount;
	static int preallocBlocks;
	static int requestedBlocks;
	void printDebug();
private:
	static StreamManager* instance;
	BOOL EnableLockPrivileges();
	static LONG WINAPI handler(PEXCEPTION_POINTERS info);
	VortexS* inputStream;
	VortexS* outputStream;
	uint64_t numStreams;
	uint64_t sizeStreamPower;
	uint64_t sizeBlockPower;
	VortexS** streams;
	StreamPool* blockPool;
	std::unordered_map<ULONG_PTR, VortexS*> startAddressToStream;
	std::unordered_map<ULONG_PTR, VortexS*> endAddressToStream;
	std::set<ULONG_PTR> intervalTree;
	
};