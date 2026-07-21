#include "VortexS.h"
#include "StreamManager.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <thread>
VortexS::VortexS(uint64_t sizeStreamPower, StreamPool* blockPool) {

	uint64_t allocSize = 8ULL * (1ULL << sizeStreamPower);
	this->startPtr = VirtualAlloc(NULL,allocSize , MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
	this->endPtr = (void*)((char*)this->startPtr + allocSize);
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

void query(ULONG_PTR ptr) {
	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery((void*)(ptr), &mbi, 1 << 12)) {
		std::cout << "virtual query failed";
		std::cout << GetLastError();
		exit(-1);
	}
}

LONG VortexS::handle_exception(PEXCEPTION_POINTERS info) {

	bool isAccessViolation = info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION;
	bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
	if (blockPool->ptrToPFN.contains((void*)0x00000229BE4FFFE8)) {
		std::cout << blockPool->ptrToPFN[(void*)0x00000229BE4FFFE8];
	}
	if (blockPool->ptrToPFN.contains((void*)0x00000229BE4FF000)) {
		std::cout << blockPool->ptrToPFN[(void*)0x00000229BE4FF000];
	}
	if (!isAccessViolation) {
		std::cout << "Not a access violation...\n";
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if (isWriteFault) {


		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		//how did my method work when this wasnt here ? even wiuth this fixed, the block counts are still occasionally off//////////////////
		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery((void*)(fptr), &mbi, sizeof(mbi))) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();	
			exit(-1);
		}
		///////////////////
		if (mbi.Protect !=  NULL) {
			//std::cout << "is this possible ? "; //i guess it is ? 
			//std::cout << std::hex << mbi.Protect << std::endl;
			removeGuardPage(fptr);
		}
		else {
			blockPool->mapBlockFromPool(fptr);
		}
		if (fptr >> 12 != (ULONG_PTR)startPtr >> 12) { // do i need this bitshift? does it always perfectly align
			uint64_t blockSizeBytes = 1ULL << sizeBlockPower;
			setGuardPage(fptr - blockSizeBytes);
		}
		
	}
	else {
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		uint64_t blockSizeBytes = 1ULL << blockPool->getSizeBlockPower();

		if (lastReadFault == -1) {
			removeGuardPage(fptr);
		}
		else if (fptr == lastReadFault + blockSizeBytes && isLastReadFaultValid) {
			blockPool->unmapBlockToPool(lastReadFault);
			removeGuardPage(fptr);
		}
		else {
			removeGuardPage(fptr);
		}
		lastReadFault = fptr;
		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery((void*)(fptr), &mbi, sizeof(mbi))) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}

		//StreamManager::instance->printDebug();
		if (!VirtualQuery((void*)(fptr + blockSizeBytes), &mbi, sizeof(mbi))) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}
		isLastReadFaultValid = mbi.Protect == PAGE_NOACCESS;
	}
	//if (StreamManager::guardCount < 0 and StreamManager::guardCount % 10000 == 0) {
	//	StreamManager::instance->printDebug();
	//	std::cout << StreamManager::guardCount << " ";
	//}

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
	if (!VirtualProtect((void*)ptr, 1, PAGE_NOACCESS, &oldProtect)) {
		std::cout << "virtual protect failed";
		std::cout << GetLastError();
		exit(-1);
	}
	StreamManager::guardCount++;
	return oldProtect;
}


DWORD VortexS::removeGuardPage(ULONG_PTR ptr) {
	uint64_t mask = ~((1 << 12) - 1);
	ptr &= mask;
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)ptr, 1, PAGE_READWRITE, &oldProtect)) {
		std::cout << "virtual protect failed";
		std::cout << GetLastError();
		exit(-1);
	}
	StreamManager::guardCount--;
	return oldProtect;
	/*needed for both release mode and debug on laptop ? non deterministic
	//MEMORY_BASIC_INFORMATION mbi;
	//if (!VirtualQuery((void*)(ptr), &mbi, 1 << 12)) {
	//	std::cout << "virtual query failed";
	//	std::cout << GetLastError();
	//	exit(-1);
	//}*/
}

std::vector<int> VortexS::blocksLeftBehind() {
	std::vector<int> blocks;
	uint64_t numBlocks = (1 << (sizeStreamPower - sizeBlockPower)) / 128;
	for (uint64_t i = 0; i < numBlocks; i++) {
		MEMORY_BASIC_INFORMATION mbi;
		void* ptr = (void*)(getStartPtr() + i * (1ULL << sizeBlockPower));
		if (!VirtualQuery(ptr, &mbi, sizeof(mbi))) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}
		if (mbi.State != MEM_RESERVE) { // when a physical page is mapped to a virtual address the memory status becomes mem_commit
			blocks.push_back(i);
		}
		
	}
	return blocks;
}
void VortexS::blocksLeftBehindThread(std::vector<int>& blocks) {
	uint64_t numBlocks = (1ULL << (sizeStreamPower - sizeBlockPower));
	for (uint64_t i = 0; i < numBlocks; i++) {
		MEMORY_BASIC_INFORMATION mbi;
		void* ptr = (void*)(getStartPtr() + i * (1ULL << sizeBlockPower));
		if (!VirtualQuery(ptr, &mbi, sizeof(mbi))) {
			std::cout << "virtual query failed";
			std::cout << GetLastError();
			exit(-1);
		}
		if (mbi.State != MEM_RESERVE) { // when a physical page is mapped to a virtual address the memory status becomes mem_commit
			blocks.push_back(i);
		}

	}
}


