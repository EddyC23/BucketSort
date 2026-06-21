#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include "BucketSort.h"
#include <chrono>
#include "StreamManager.h"
#include <chrono>
int main() {
	if (true) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint64_t> distribution(0);

		uint64_t numStreams = 258; // 1 input stream, 1 output stream, 256 bucket streams
		uint64_t sizeStreamPower = 27; 
		uint64_t sizeBlockPower = 13;
		uint64_t additionalBlocks =  256 * 4 + (1 << 8); 

		StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
		uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
		uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();
		for (size_t i = 0; i < 1ULL << (sizeStreamPower - 3); i++) {
			//input[i] = distribution(gen);
			uint64_t mask = ~(((1ULL << 8) - 1) << 48);
			input[i] = distribution(gen) & mask;
		}
		std::cout << "Input Done.\nSize Stream in Blocks : " << (1ULL << (sizeStreamPower - sizeBlockPower)) << "\n";
		//std::cout << StreamManager::mapCount << " " << StreamManager::unmapCount;
		int levelFlag = 10; // stops before this level of recursion (0 stops before any work is done)
		BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), levelFlag);
		b.sort();
		b.isSorted();
		sm.printDebug();
		

		
		
	}
	//else{
	//	std::random_device rd;
	//	std::mt19937 gen(rd());
	//	std::binomial_distribution<uint64_t> distribution(UINT64_MAX, 0.5);

	//	uint64_t numStreams = 1 + 1 + (1 << 8); // 1 input stream, 1 output stream, 256 bucket streams
	//	uint64_t sizeStreamPower = 3 + 27; // 1 gb input
	//	uint64_t sizeBlockPower = 20; // 1 mb blocks
	//	uint64_t additionalBlocks = 256 * 4 + (1 << 8); // 2048 + 256buckets // two blocks lost per level?

	//	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	//	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	//	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();
	//	for (size_t i = 0; i < 1ULL << (sizeStreamPower - 3); i++) {
	//		input[i] = distribution(gen);
	//	}
	//	std::cout << "Input Done.\nSize Stream in Blocks : " << (1ULL << (sizeStreamPower - sizeBlockPower)) << "\n";

	//	BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), 1000);
	//	b.sort();
	//	std::cout << "Sorting Done\n";
	//	b.isSorted();
	//}
}
// consolidate virtual alloc to stream manager vortexS
//for uniform case break it down into levels and find out where blocks are being lost and reduce amount of blocks that are extra
// after finishing a bucket, reset the bucket, and reset the L pointer visualization of where the blocks are lost, patterns, curios from 518 to
// have a per bucket linear search through to see where the blocks are lost see where the jump from 2^24 to 2^27 to 2^30, with 2^14 pages
// save first block later and set the L1 to point to the beginning of a bucket