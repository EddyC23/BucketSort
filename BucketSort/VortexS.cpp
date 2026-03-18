#include "VortexS.h"
#include <cstdio>
#include <iostream>
VortexS::VortexS(uint64_t sizeStreamPower, StreamPool* blockPool) {
	this->startPtr = VirtualAlloc(NULL, 1ULL << sizeStreamPower, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
	this->endPtr = (void*)((char*)this->startPtr + (1ULL << sizeStreamPower));
	this->sizeStreamPower = sizeStreamPower;
	this->sizeBlockPower = blockPool->getSizeBlockPower();
	this->blockPool = blockPool;
	this->lastReadFault = -1;
	this->isLastReadFaultValid = false;
	if (this->startPtr == NULL) {
		std::cout << "virtual alloc failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
LONG VortexS::handle_exception(PEXCEPTION_POINTERS info) {
	bool isGuardViolation = info->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE;
	bool isAccessViolation = info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION;
	if (!isAccessViolation && !isGuardViolation) {
		std::cout << "Not a access violation or guard violation...\n";
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if (isGuardViolation) {
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		uint64_t blockSizeBytes = 1ULL << blockPool->getSizeBlockPower();

		if (lastReadFault != -1 && fptr == lastReadFault + blockSizeBytes && isLastReadFaultValid) {
			blockPool->unmapBlockToPool(lastReadFault);
		}
		lastReadFault = fptr;
		MEMORY_BASIC_INFORMATION memInfo;
		if (!VirtualQuery((void*)(fptr + blockSizeBytes), &memInfo, 1 << 12)) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}
		std::cout << "Is Valid XDD" << isLastReadFaultValid;
		isLastReadFaultValid = memInfo.AllocationProtect & PAGE_GUARD;
	} 
	else if (isAccessViolation) {
		bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		if (isWriteFault) {
			blockPool->mapBlockFromPool(fptr);
			printf("%llx %llx %d %llx", fptr >> sizeBlockPower, ((ULONG_PTR)startPtr) >> sizeBlockPower, (fptr >> sizeBlockPower) - (((ULONG_PTR)startPtr) >> sizeBlockPower), fptr);
			if (fptr >> sizeBlockPower != ((ULONG_PTR)startPtr) >> sizeBlockPower) { // if its not the first block
				uint64_t blockSizeBytes = 1ULL << sizeBlockPower;
				setGuardPage(fptr - blockSizeBytes);
				std::cout << "Made guard page!\n";
			}
		}
		else {
			std::cout << "only guard page violation should be triggered for read faults";
			std::cout << GetLastError();
			exit(-1);
		}
	}
	return EXCEPTION_CONTINUE_EXECUTION;
}
ULONG_PTR VortexS::getStartPtr() {
	return (ULONG_PTR)startPtr;
}
ULONG_PTR VortexS::getEndPtr() {
	return (ULONG_PTR)endPtr;
}
DWORD VortexS::setGuardPage(ULONG_PTR ptr) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)ptr, 1 << 12, PAGE_NOACCESS, &oldProtect)) {
		std::cout << "virtual protect failed";
		std::cout << GetLastError();
		exit(-1);
	}

	MEMORY_BASIC_INFORMATION memInfo;
	if (!VirtualQuery((void*)(ptr ), &memInfo, 1 << 12)) {
		std::cout << "virtual query failed";
		std::cout << GetLastError();
		exit(-1);
	}
	
	printf("Alloc protect %lx", memInfo.AllocationProtect);
	return oldProtect;
}