#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include "BucketSort.h"



int main() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint64_t> distribution(0);

	uint64_t* input = new uint64_t[1 << 16];
	uint64_t* output = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = distribution(gen);
	}

	BucketSort b(input, output, 1 << 16);
	b.sort();
	b.printArray(output, 1 << 16);
	std::cout << b.isSorted();
}