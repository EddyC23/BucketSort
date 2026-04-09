#include "StreamManager.h"
#include <iostream>


StreamManager* StreamManager::instance = nullptr;

int StreamManager::guardCount = 0;
int StreamManager::mapCount = 0;
int StreamManager::unmapCount = 0;
int StreamManager::blocksNeededCount = 0;
int StreamManager::preallocBlocks = 0;
int StreamManager::requestedBlocks = 0;

StreamManager::StreamManager(uint64_t numStreams, uint64_t sizeStreamPower, uint64_t sizeBlockPower, uint64_t additionalBlocks) {
	
	EnableLockPrivileges();
	if (AddVectoredExceptionHandler(1, handler) == NULL) {
		std::cout << "add vectored exception handler failed";
		std::cout << GetLastError();
		exit(-1);
	}

	instance = this;
	this->sizeStreamPower = sizeStreamPower;
	this->sizeBlockPower = sizeBlockPower;
	this->numStreams = numStreams;
	
	StreamManager::preallocBlocks = (1ULL << (sizeStreamPower - sizeBlockPower)) * 256.0 / 255.0 + additionalBlocks;
	this->blockPool = new StreamPool(StreamManager::preallocBlocks, sizeBlockPower);
	
	this->inputStream = new VortexS(sizeStreamPower, blockPool);
	this->outputStream = new VortexS(sizeStreamPower, blockPool);
	this->streams = new VortexS * [numStreams];

	

	if (numStreams < 2) {
		std::cout << "not enough streams";
		std::cout << GetLastError();
		exit(-1);
	}
	this->streams[0] = inputStream; 
	this->streams[1] = outputStream;
	
	for (size_t i = 2; i < numStreams; i++) {
		streams[i] = new VortexS(sizeStreamPower, blockPool);
	}
}
LONG WINAPI StreamManager::handler(PEXCEPTION_POINTERS info) {
	bool isAccessViolation = info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION;
	if (isAccessViolation) {
		VortexS* streamPtr = instance->getStreamFromAddressLinear((ULONG_PTR)(info->ExceptionRecord->ExceptionInformation[1]));
		
		if (streamPtr == nullptr) {
			std::cout << "Stream not found...\n";
		}
		return streamPtr->handle_exception(info);
	}
	return 0;
}
VortexS* StreamManager::getStreamFromAddressLinear(ULONG_PTR faultAddress) {
	for (uint64_t i = 0; i < numStreams; i++) {
		if (faultAddress >= streams[i]->getStartPtr() && faultAddress < streams[i]->getEndPtr()) {
			return *(streams + i);
		}
	}
	return nullptr;
}
VortexS* StreamManager::getInputStream() {
	return this->inputStream;
}
VortexS* StreamManager::getOutputStream() {
	return this->outputStream;
}
VortexS* StreamManager::getNthStream(int n) {
	return streams[n + 2];
}
void StreamManager::printDebug() {
	std::cout << "Size Stream in Blocks : " << (1ULL << (sizeStreamPower - sizeBlockPower)) << "\n";
	std::cout << "Total Block Map Count : " << mapCount << "\n";
	std::cout << "Total Block Unmap Count : " << unmapCount << "\n";
	std::cout << "Blocks Left Behind : " << mapCount - unmapCount << "\n";
	std::cout << "Total Blocks Needed For Sort: " << blocksNeededCount << "\n";
	std::cout << "Blocks Needed / Blocks Allocated : " << (blocksNeededCount + 0.0)/(preallocBlocks + requestedBlocks) << "\n";
	std::cout << "Guard Pages Left After Sort: " << guardCount << "\n";
}

BOOL StreamManager::EnableLockPrivileges() {
	//sets enable lock privileges
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	// 1. Open the process token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken failed. Error: %lu\n", GetLastError());
		return FALSE;
	}

	// 2. Get the LUID for "SeLockMemoryPrivilege"
	if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid)) {
		printf("LookupPrivilegeValue failed. Error: %lu\n", GetLastError());
		CloseHandle(hToken);
		return FALSE;
	}

	// 3. Enable the privilege
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		printf("AdjustTokenPrivileges failed. Error: %lu\n", GetLastError());
		CloseHandle(hToken);
		return FALSE;
	}

	// 4. Check if it actually worked (AdjustTokenPrivileges returns TRUE even if it failed to add the right)
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("The token does not have the specified privilege. \n");
		printf("PLEASE NOTE: You must grant 'Lock pages in memory' in Local Security Policy (secpol.msc) first!\n");
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}