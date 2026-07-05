#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include <chrono>
#include "BucketSort.h"
#include "StreamManager.h"
#include "writer.h"

using clk = std::chrono::steady_clock;
const double ONE_BILLION = 1000000000;
const double ONE_MILLION = 1000000;

uint64_t keysPerSecond(uint64_t keyCount, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::time_point<std::chrono::steady_clock> end) {
	auto durationInNs = end - start;
	double seconds = (durationInNs.count() + 0.0) / ONE_BILLION;
	return keyCount / seconds;
}

int main() {
	auto programStart = clk::now();

	uint64_t numStreams = 258; // 1 input stream, 1 output stream, 256 bucket streams
	uint64_t sizeStreamPower = 30; 
	uint64_t sizeBlockPower = 13;
	uint64_t additionalBlocks =  256 * 4 + (1 << 8); 
	uint64_t numberKeys = 1ULL << (sizeStreamPower - 3);

	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();

	datagen::Writer<uint64_t> writer;
	writer.generate(input, numberKeys, datagen::UNIFORM_DBL);
	std::cout << "Input done.\n";
	
	int stopRecursionLevel= 10; 
	BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), stopRecursionLevel);
	std::cout << "Beginning sort.\n";

	auto start = clk::now();
	b.sort();
	auto end = clk::now();
	
	b.isSorted();
	sm.printDebug();
	uint64_t keyCount = 1ULL << (sizeStreamPower - 3);
	std::cout <<"\nSort Speed : " <<  keysPerSecond(keyCount, start, end) / ONE_MILLION << " million keys/s\n\n\n\n\n";

	auto programEnd = clk::now();
	double programTimeSeconds = (programEnd - programStart).count() / ONE_BILLION;
	std::cout << std::format("The program executed in {} seconds ... ", programTimeSeconds);
	
}