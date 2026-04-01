#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include "BucketSort.h"
#include <chrono>
#include "StreamManager.h"



int main() {	
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint64_t> distribution(0);

	uint64_t numStreams = 1 + 1 + (1 << 8); // 1 input stream, 1 output stream, 2 ^ 8 buckets
	uint64_t sizeStreamPower = 3 + 27; // 64-bit integer 2 ^ 3 = 8 bytes, 2 ^ 20 integers
	uint64_t sizeBlockPower = 13; // 2 ^ 13 bytes, 8 kb blocks
	uint64_t additionalBlocks = 1 << 8; // 2 ^ 8 buckets
	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();
	for (size_t i = 0; i < 1ULL << (sizeStreamPower - 3); i++) {
		input[i] = distribution(gen);
	}
	std::cout << "Input Done.\nSize Stream in Blocks : " << (1ULL << (sizeStreamPower - sizeBlockPower)) << "\n";

	BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), 1000);
	b.sort();
	std::cout << "Sorting Done\n";
	std::cout << b.isSorted();
	
	//	//static preallocated buckets n/256 only for L0 for both
	//write onyl commit only stream (stream)
}
//correctness for data generator
// gb sort
//fix mmeory overhead 1 + 1/255 + 2k ish 256 * levels worst case // take back blocks two st
//if exhaust request mroe blocks