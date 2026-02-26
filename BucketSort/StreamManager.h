#pragma once
#include "VortexS.h"
#include "StreamPool.h"
#include <unordered_map>
class StreamManager {
public:
	StreamManager(uint64_t numStreams, uint64_t sizeStreamPower, uint64_t sizeBlockPower, uint64_t additionalBlocks);
	VortexS* getInputStream();
private:
	static StreamManager* instance;
	BOOL EnableLockPrivileges();
	static LONG WINAPI handler(PEXCEPTION_POINTERS info);
	VortexS** streams;
	VortexS* inputStream;
	uint64_t numStreams;
	uint64_t sizeStreamPower;
	//std::unordered_map<ULONG_PTR, VortexS*> startAddressToStream;
	VortexS* getStreamFromAddress(ULONG_PTR faultAddress);
	
};