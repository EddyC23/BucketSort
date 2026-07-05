#include "StreamManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "StreamPool.h"
StreamManager* StreamManager::instance = nullptr;

int StreamManager::guardCount = 0;
int StreamManager::mapCount = 0;
int StreamManager::unmapCount = 0;
int StreamManager::blocksNeededCount = 0;
int StreamManager::preallocBlocks = 0;
int StreamManager::requestedBlocks = 0;
int StreamManager::helper = 0;
StreamManager::StreamManager(uint64_t numStreams, uint64_t sizeStreamPower, uint64_t sizeBlockPower, uint64_t additionalBlocks) {
	if (!EnableLockPrivileges()) {
		std::cout << "enable lock privileges failed";
		std::cout << GetLastError();
		exit(-1);
	}
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
	this->blockPool = new StreamPool(StreamManager::preallocBlocks, sizeBlockPower, this);
	
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
int StreamManager::getStreamIndexFromAddressLinear(ULONG_PTR faultAddress) {
	for (int i = 0; i < numStreams; i++) {
		if (faultAddress >= streams[i]->getStartPtr() && faultAddress < streams[i]->getEndPtr()) {
			return i;
		}
	}
	return -1;
}
VortexS* StreamManager::getInputStream() {
	return this->inputStream;
}
VortexS* StreamManager::getOutputStream() {
	return this->outputStream;
}
VortexS* StreamManager::getBucketStream(int n) {
	return streams[n + 2];
}
VortexS* StreamManager::getNthStream(int n) {
	return streams[n];
}
void StreamManager::printDebug() {
	uint64_t sizeStreamBlock = (1ULL << (sizeStreamPower - sizeBlockPower));
	std::cout << "Size Stream in Blocks : " << sizeStreamBlock << "\n";
	std::cout << "Total Block Map Count : " << mapCount << "\n";
	std::cout << "Total Block Unmap Count : " << unmapCount << "\n";
	std::cout << "Max Block Usage During Sort: " << blocksNeededCount << " (" << (blocksNeededCount + 0.0) * 100 / sizeStreamBlock << "% of stream size)\n";
	std::cout << "Blocks Left Behind : " << mapCount - unmapCount << "\n\n";
	std::chrono::steady_clock clk;
	auto t1 = clk.now();
	std::vector<std::vector<int>> blocksLeft = getBlocksLeftBehindThread(); //343 ms
	auto t2 = clk.now();
	std::cout << "Time to find blocks left behind : " << (std::chrono::duration_cast<std::chrono::milliseconds>)(t2 - t1) << "\n";
	int counter = 0;
	for (int i = 0; i < blocksLeft.size(); i++) {
		std::string s = "";
		s += std::to_string(i) + " : ";
		for (int j = 0; j < blocksLeft[i].size(); j++) {
			s += std::to_string(blocksLeft[i][j]) + " ";
			counter++;
		}
		std::cout << std::left << std::setw(50) << s << "|" << blocksLeft[i].size() << "blocks\n";
	}
	std::cout << std::format("\n{} blocks counted left behind from buckets\n2 blocks left behind from input stream\n2 blocks left behind from output stream\n{} blocks left behind total\n", counter, 4 + counter); // the 4 is from the 2 blocks trivially left in the input and output stream we are losing one extra block somewhere in there
}

std::vector<std::vector<int>> StreamManager::getBlocksLeftBehindThread() {
	std::vector<std::vector<int>> blocksLeft(256);
	std::thread* threadPtrs[256];
	for (size_t i = 2; i < numStreams; i++) {
		threadPtrs[i - 2] = new std::thread(&StreamPool::getBlocksLeftBehindThread, blockPool, i-2, std::ref(blocksLeft.at(i - 2)));
	}
	for (size_t i = 0; i < 256; i++) {
		threadPtrs[i]->join();
	}
	
	return blocksLeft;
}

void StreamManager::cleanUpBlocks(int i) {
	// frees all blocks that are left still behind after a stream is done, leaves behind the first block(will need to reset pointer for this) ?
	blockPool->cleanUpBlocks(i);
}

void StreamManager::mapBlockFromPool(ULONG_PTR ptr) {
	blockPool->mapBlockFromPool(ptr);
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