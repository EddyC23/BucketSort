#include <iostream>
#include <algorithm>
#include <windows.h>
#include "BucketSort.h"

void printArray(uint64_t* ptr, uint64_t size) {
	for (size_t i = 0; i < size; i++) {
		std::cout << "index" << i << " : " << ptr[i] << "\n";
	}
}

int main() {
	uint64_t* input = new uint64_t[1 << 16];
	uint64_t* output = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = (1 << 16) - i;
		
	}

	
	BucketSort b(input, output, 1 << 16);
	b.sort();
	printArray(output, 1 << 16);

	
	
}