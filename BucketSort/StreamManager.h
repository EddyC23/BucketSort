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
private:
	static StreamManager* instance;
	BOOL EnableLockPrivileges();
	static LONG WINAPI handler(PEXCEPTION_POINTERS info);
	VortexS* inputStream;
	VortexS* outputStream;
	uint64_t numStreams;
	uint64_t sizeStreamPower;
	VortexS** streams;
	std::unordered_map<ULONG_PTR, VortexS*> startAddressToStream;
	std::unordered_map<ULONG_PTR, VortexS*> endAddressToStream;
	std::set<ULONG_PTR> intervalTree;
	
};