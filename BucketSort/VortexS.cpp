#include "VortexS.h"
#include <cstdio>
#include <iostream>
int count = 0;
int mapCount = 0;
int unmapCount = 0;
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

//helper


void query(ULONG_PTR ptr) {
	MEMORY_BASIC_INFORMATION memInfo;
	if (!VirtualQuery((void*)(ptr), &memInfo, 1 << 12)) {
		std::cout << "virtual query failed";
		std::cout << GetLastError();
		exit(-1);
	}
	//std::cout << memInfo.
}

LONG VortexS::handle_exception(PEXCEPTION_POINTERS info) {
	bool isAccessViolation = info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION;
	bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
	if (!isAccessViolation) {
		std::cout << "Not a access violation...\n";
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if (isWriteFault) {
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		blockPool->mapBlockFromPool(fptr);
		if (fptr >> sizeBlockPower != ((ULONG_PTR)startPtr) >> sizeBlockPower) { // if its not the first block
			uint64_t blockSizeBytes = 1ULL << sizeBlockPower;
			setGuardPage(fptr - blockSizeBytes);
			std::cout << "Made guard page!";
			std::cout << ++count << "\n";
		}
	}
	else {
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		uint64_t blockSizeBytes = 1ULL << blockPool->getSizeBlockPower();
		if (lastReadFault == -1) {
			removeGuardPage(fptr);
			std::cout << "Removed guard page!";
			std::cout << --count << "\n";
		}
		else if (fptr == lastReadFault + blockSizeBytes && isLastReadFaultValid) {
			blockPool->unmapBlockToPool(lastReadFault);
			removeGuardPage(fptr);
			std::cout << "Removed guard page!";
			std::cout << --count << "\n";
		}
		lastReadFault = fptr;
		MEMORY_BASIC_INFORMATION memInfo;
		if (!VirtualQuery((void*)(fptr + blockSizeBytes), &memInfo, 1 << 12)) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}
		isLastReadFaultValid = memInfo.Protect == PAGE_NOACCESS;
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
	
	
	//std::cout << GetLastError();
	//printf("Old protect %lx \nAlloc protect %lx \n",oldProtect, memInfo.Protect);
	return oldProtect;
}
DWORD VortexS::removeGuardPage(ULONG_PTR ptr) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)ptr, 1 << 12, PAGE_READWRITE, &oldProtect)) {
		std::cout << "virtual protect failed";
		std::cout << GetLastError();
		exit(-1);
	}
	MEMORY_BASIC_INFORMATION memInfo;
	if (!VirtualQuery((void*)(ptr), &memInfo, 1 << 12)) {
		std::cout << "virtual query failed";
		std::cout << GetLastError();
		exit(-1);
	}


	//std::cout << GetLastError();
	printf("Old protect %lx \nAlloc protect %lx \n",oldProtect, memInfo.Protect);
	return oldProtect;

}