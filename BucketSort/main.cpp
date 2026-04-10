#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include "BucketSort.h"
#include <chrono>
#include "StreamManager.h"

//enum Distribution {
//	UNIFORM = 0,
//	NORMAL = 1,
//	EXPONENTIAL = 2
//};

int main() {
	//void* ptr = VirtualAlloc(NULL, 1ULL << 42, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	//printf("%llx", ptr);
	//return 0;
	if (true) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint64_t> distribution(0);

		uint64_t numStreams = 1 + 1 + (1 << 8); // 1 input stream, 1 output stream, 256 bucket streams
		uint64_t sizeStreamPower = 3 + 27; // 1 gb input
		uint64_t sizeBlockPower = 20;// 1 mb blocks
		uint64_t additionalBlocks =  256 * 4 + (1 << 8); // 2048 + 256buckets // two blocks lost per level?

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
		b.isSorted();
	}
	else{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::binomial_distribution<uint64_t> distribution(UINT64_MAX, 0.5);

		uint64_t numStreams = 1 + 1 + (1 << 8); // 1 input stream, 1 output stream, 256 bucket streams
		uint64_t sizeStreamPower = 3 + 27; // 1 gb input
		uint64_t sizeBlockPower = 20; // 1 mb blocks
		uint64_t additionalBlocks = 256 * 4 + (1 << 8); // 2048 + 256buckets // two blocks lost per level?

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
		b.isSorted();
	}
}
//correctness for data generator
// gb sort
//fix mmeory overhead 1 + 1/255 + 2k ish 256 * levels worst case // take back blocks two st
//if exhaust request mroe blocks
//correctness, data generator for other distributions, adding blocks when needed monitor how many are needed, 
// consolidate virtual alloc to stream manager vortexS
//for uniform case break it down into levels and find out where blocks are being lost and reduce amount of blocks that are extra