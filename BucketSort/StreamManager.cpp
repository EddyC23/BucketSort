#include "StreamManager.h"
#include <iostream>

StreamManager* StreamManager::instance = nullptr;

StreamManager::StreamManager(uint64_t numStreams, uint64_t sizeStreamPower, uint64_t sizeBlockPower, uint64_t additionalBlocks) {
	
	EnableLockPrivileges();
	if (AddVectoredExceptionHandler(1, handler) == NULL) {
		std::cout << "add vectored exception handler failed";
		std::cout << GetLastError();
		exit(-1);
	}

	StreamPool* blockPool = new StreamPool(sizeStreamPower, sizeBlockPower, additionalBlocks);
	instance = this;
	this->sizeStreamPower = sizeStreamPower;
	this->numStreams = numStreams;
	this->inputStream = new VortexS(sizeStreamPower, blockPool);
	this->streams = new VortexS * [numStreams];
	this->streams[0] = inputStream;
	for (size_t i = 1; i < numStreams; i++) {
		streams[i] = new VortexS(sizeStreamPower, blockPool);	
	}
	int x = 5;

}
LONG WINAPI StreamManager::handler(PEXCEPTION_POINTERS info) {
	VortexS* streamPtr = instance->getStreamFromAddress((ULONG_PTR)(info->ExceptionRecord->ExceptionAddress));
	if (streamPtr == nullptr) {
		std::cout << "Stream not found...\n";
		exit(-1);
	}
	return streamPtr->handle_exception(info);
}
VortexS* StreamManager::getStreamFromAddress(ULONG_PTR faultAddress) {
	for (uint64_t i = 0; i < numStreams; i++) {
		std::cout << "Start : " << streams[i]->getStartPtr() << " END : " << streams[i]->getEndPtr() << "\n";
	}
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
