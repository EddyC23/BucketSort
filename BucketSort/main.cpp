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

	uint64_t numStreams = 2 + (1 << 8); // 1 (input stream) + 2 ^ 8 buckets
	uint64_t sizeStreamPower = 3 + 20;
	uint64_t sizeBlockPower = 12;
	//uint64_t additionalBlocks = (1ULL << (19 - 12)) + (1 << 8);//should be 2^8, append only right now though
	uint64_t additionalBlocks = 1 << 8;
	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	//static preallocated buckets n/256 only for L0 for both
	//write onyl commit only stream (stream)
	//

	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();
	for (size_t i = 0; i < 1 << 20; i++) {
		input[i] = distribution(gen);
	}
	std::cout << "Input Done.";
	BucketSort b(input, output, 1 << 20, 100);
	b.sort();
	//b.printArray(output, 1 << 16);
	std::cout << b.isSorted();
	{
		/*
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint64_t> distribution(0);

	uint64_t* input = new uint64_t[1 << 16];
	uint64_t* output = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {~
		input[i] = distribution(gen);
	}
	BucketSort b(input, output, 1 << 16, 0);~
	b.sort();
	b.printArray(output, 1 << 16);
	std::cout << b.isSorted();
	//flag stops after different levels eg L0 l1 to time benchmark
	//L0 with vortex S stream 5.3, append only can improve, corner cases
	//
	//interval tree later
	//flag as parameter
	//static is ceiling
	*/
	/*
	// Benchmarking code : 100 000 000 accesses in random streams
	uint64_t numStreams = 1 + (1 << 8); // inputstream + 2^8 bucket
	uint64_t sizeStreamPower = 3 + 16;
	uint64_t sizeBlockPower = 12;
	uint64_t additionalBlocks = (1ULL << (19 - 12)) + 8;//should be 8, append only right now though
	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);

	uint64_t steps = 0;
	uint64_t interval = 1 << 10;

	srand(static_cast<unsigned int>(time(0)));

	uint64_t numAccesses = 100000000;
	double timeTotalMs = 0;
	clock_t startClock = clock();
	for (uint64_t i = 0; i < numAccesses; i++) {
		int index = rand() % numStreams;
		sm.getStreamFromAddressLinear(sm.testStreams[index] + rand() % numAccesses);
	}
	clock_t endClock = clock();
	timeTotalMs += endClock - startClock;
	std::cout << "Linear Time Per Call : " << timeTotalMs << "ms";

	timeTotalMs = 0;
	startClock = clock();
	for (uint64_t i = 0; i < numAccesses; i++) {
		int index = rand() % numStreams;
		sm.getStreamFromAddressHash(sm.testStreams[index] + rand() % numAccesses);
	}
	endClock = clock();
	timeTotalMs += endClock - startClock;
	std::cout << "Hash Time Per Call : " << timeTotalMs << "ms";

	timeTotalMs = 0;
	startClock = clock();
	for (uint64_t i = 0; i < numAccesses; i++) {
		int index = rand() % numStreams;
		sm.getStreamFromAddressInterval(sm.testStreams[index] + rand() % numAccesses);
	}
	endClock = clock();
	timeTotalMs += endClock - startClock;
	std::cout << "Tree Time Per Call : " << timeTotalMs << "ms";
	*/

	}
}