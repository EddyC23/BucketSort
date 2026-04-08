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
		std::normal_distribution<double> distribution(0.0, 1.0 * UINT64_MAX);

		uint64_t numStreams = 1 + 1 + (1 << 8); // 1 input stream, 1 output stream, 256 bucket streams
		uint64_t sizeStreamPower = 3 + 27; // 1 gb input
		uint64_t sizeBlockPower = 20; // 1 mb blocks
		uint64_t additionalBlocks = 256 * 4 + (1 << 8); // 2048 + 256buckets // two blocks lost per level?

		StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
		uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
		uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();
		for (size_t i = 0; i < 1ULL << (sizeStreamPower - 3); i++) {
			input[i] = distribution(gen);
			//std::cout << input[i] << "\n";
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
