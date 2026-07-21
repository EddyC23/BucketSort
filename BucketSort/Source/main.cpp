#include <iostream>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <random>
#include <chrono>
#include <thread>
#include "BucketSort.h"
#include "StreamManager.h"
#include "writer.h"


// debug trhead stuff
#include <conio.h>
void listenForQ() {
	while (true) {
		int ch = _getch();
		if (toupper(ch) == 'Q') {
			StreamManager::instance->printDebug();
		}
	}
}
using clk = std::chrono::steady_clock;
const double ONE_BILLION = 1000000000;
const double ONE_MILLION = 1000000;

uint64_t keysPerSecond(uint64_t keyCount, std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::time_point<std::chrono::steady_clock> end) {
	auto durationInNs = end - start;
	double seconds = (durationInNs.count() + 0.0) / ONE_BILLION;
	return keyCount / seconds;
}
void writeSequentiallyIntoBuffer(uint64_t* source, uint64_t* destination, uint64_t length) {
	for (uint64_t i = 0; i < length; i++) {
		destination[i] = source[i];
	}

}

void runAll() {
	std::vector<datagen::WRITER_TYPE> allWriterTypes;
	{
		using namespace datagen;
		std::vector<WRITER_TYPE> temp{ MT,
										ALL_SAME,
										SORTED,
										REV_SORTED,
										ALMOST_SORTED,
										//PARETO_NONUNIFORM, // broken
										PARETO_B2B,
										PARETO_SHUFF,
										FIB,
										NORMAL,
										UNIFORM_DBL,
										MID_ZEROS,
										RANDOM_PERM_N,
										CONST_RAND,
										CRAND_GAUSSIAN,
										WORST_CASE,
										WORST_CASE2,
										PD, // broken
										UD,
										U_SEQ,
										R_SEQ,
										WORST_CASE_BACKSCAN,
										WORST_CASE_FSCAN,
										 WORST_CASE_QSORT, // broken
										WC_ADVERSARIAL_MSD,
										WC_ADVERSARIAL_LSD,
										PD16,
										PD64,
										PD512, };
		allWriterTypes.insert(allWriterTypes.begin(), temp.begin(), temp.end());
		}
	uint64_t numStreams = 258; // 1 input stream, 1 output stream, 256 bucket streams
	uint64_t sizeStreamPower = 25;
	uint64_t sizeBlockPower = 16;
	uint64_t additionalBlocks = 256 * 4 + (1 << 8);
	uint64_t numberKeys = 1ULL << (sizeStreamPower - 3);
	for (datagen::WRITER_TYPE writerType : allWriterTypes) {
		uint64_t* temp = new uint64_t[numberKeys];

		std::cout << writerType << "\n";

		StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
		uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
		uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();

		datagen::Writer<uint64_t> writer;
		writer.generate(temp, numberKeys, writerType);
		writeSequentiallyIntoBuffer(temp, input, numberKeys);
		std::cout << "Input done.\n";

		int stopRecursionLevel = 10;
		BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), stopRecursionLevel);
		std::cout << "Beginning sort.\n";

		auto start = clk::now();
		b.sort();
		auto end = clk::now();

		b.isSorted();
		//sm.printDebug();
		uint64_t keyCount = 1ULL << (sizeStreamPower - 3);
		std::cout << "\nSort Speed : " << keysPerSecond(keyCount, start, end) / ONE_MILLION << " million keys/s\n\n\n\n\n";
		delete[] temp;
	}
	//delete[] temp;
}

void runOne(datagen::WRITER_TYPE writerType) {
	uint64_t numStreams = 258; // 1 input stream, 1 output stream, 256 bucket streams
	uint64_t sizeStreamPower = 30;
	uint64_t sizeBlockPower = 20;
	//uint64_t additionalBlocks = 256 * 4 + (1 << 8);
	uint64_t additionalBlocks = 0;
	uint64_t numberKeys = 1ULL << (sizeStreamPower - 3);
	uint64_t* temp = new uint64_t[numberKeys];

	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();

	datagen::Writer<uint64_t> writer;
	writer.generate(temp, numberKeys, writerType);
	writeSequentiallyIntoBuffer(temp, input, numberKeys);
	delete[] temp;
	std::cout << "Input done.\n";

	int stopRecursionLevel = 10;
	BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), stopRecursionLevel);
	std::cout << "Beginning sort.\n";

	auto start = clk::now();
	b.sort();
	auto end = clk::now();

	b.isSorted();
	sm.printDebug();
	uint64_t keyCount = 1ULL << (sizeStreamPower - 3);
	std::cout << "\nSort Speed : " << keysPerSecond(keyCount, start, end) / ONE_MILLION << " million keys/s\n\n\n\n\n";

}

void runCustom() {
	uint64_t numStreams = 258; // 1 input stream, 1 output stream, 256 bucket streams
	uint64_t sizeStreamPower = 30;
	uint64_t sizeBlockPower = 20;
	//uint64_t additionalBlocks = 256 * 4 + (1 << 8);
	uint64_t additionalBlocks = 0;
	uint64_t numberKeys = 1ULL << (sizeStreamPower - 3);
	uint64_t* temp = new uint64_t[numberKeys];

	StreamManager sm(numStreams, sizeStreamPower, sizeBlockPower, additionalBlocks);
	uint64_t* input = (uint64_t*)sm.getInputStream()->getStartPtr();
	uint64_t* output = (uint64_t*)sm.getOutputStream()->getStartPtr();

	for (uint64_t i = 0; i < numberKeys; i++) {
		input[i] = i << 30;
	}

	std::cout << "Input done.\n";

	int stopRecursionLevel = 10;
	BucketSort b(&sm, input, output, 1ULL << (sizeStreamPower - 3), stopRecursionLevel);
	std::cout << "Beginning sort.\n";

	auto start = clk::now();
	b.sort();
	auto end = clk::now();

	b.isSorted();
	sm.printDebug();
	uint64_t keyCount = 1ULL << (sizeStreamPower - 3);
	std::cout << "\nSort Speed : " << keysPerSecond(keyCount, start, end) / ONE_MILLION << " million keys/s\n\n\n\n\n";

}
int main() {
	auto programStart = clk::now();
	std::thread t1(listenForQ);
	t1.detach();
	//runOne(datagen::PARETO_NONUNIFORM); // PARETO_NONUNIFORM never works, always deadlocks
	//WORST_CASE_QSORT? PD? dont work in run all there is some shared state/corruption that is not cleand between runs for these 2? 
	runAll();
	//runCustom();
	auto programEnd = clk::now();
	double programTimeSeconds = (programEnd - programStart).count() / ONE_BILLION;
	std::cout << std::format("The program executed in {} seconds ... ", programTimeSeconds);
	
}