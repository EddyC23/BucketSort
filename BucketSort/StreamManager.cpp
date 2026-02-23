#include "StreamManager.h"
#include ""
StreamManager::StreamManager(uint64_t numStreams, uint64_t sizeStreamPower) {

	for (size_t i = 0; i < numStreams; i++) {
		VortexS* temp = new VortexS(sizeStreamPower);
		startAddressToStream[temp->startPtr] = temp;
	}
	
}
LONG StreamManager::handler(PEXCEPTION_POINTERS info) {
	info->ExceptionRecord->ExceptionInformation[1]
}
VortexS* StreamManager::getStreamFromAddress(PVOID fault) {
	ULONG_PTR ptr = ULONGPTR(fault)
	return ptr & (1ULL )
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
