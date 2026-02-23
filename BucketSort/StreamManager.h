#pragma once
#include "VortexS.h"
#include "StreamPool.h"
#include "IntervalTree.h"
#include <unordered_map>
class StreamManager {
public:
	StreamManager(uint64_t numStreams, uint64_t sizeStreams);
private:
	//struct Interval{
	//	ULONG_PTR start;
	//	ULONG_PTR end;
	//};
	IntervalTree intervals;
	BOOL EnableLockPrivileges();
	static LONG handler(PEXCEPTION_POINTERS info);
	VortexS** streams;
	std::unordered_map<ULONG_PTR, VortexS*> startAddressToStream;
	VortexS* getStream(ULONG_PTR faultAddress);
};