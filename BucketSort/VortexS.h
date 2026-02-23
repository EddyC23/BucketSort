#pragma once
#include <windows.h>
#include <cstdint>
class VortexS {
public:
	VortexS(uint64_t sizeStreamPower);
private:
	void* startPtr;
};