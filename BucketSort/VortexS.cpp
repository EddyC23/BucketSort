#include "VortexS.h"
#include <cstdio>
#include <iostream>
VortexS::VortexS(uint64_t sizeStreamPower) {
	this->startPtr = VirtualAlloc(NULL, 1ULL << sizeStreamPower, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
	this->endPtr = (void*)((char*)this->startPtr + (1ULL << sizeStreamPower));
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
		std::cout << "Not a access violation...\n";
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if (isAccessViolation) {
		bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
		ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
		if (isWriteFault) {

		}
		else {
			//should trigger guard page fault for read faults. 
		}
	}
	if (isGuardViolation) {

	}
}
ULONG_PTR VortexS::getStartPtr() {
	return (ULONG_PTR)startPtr;
}
ULONG_PTR VortexS::getEndPtr() {
	return (ULONG_PTR)endPtr;
}