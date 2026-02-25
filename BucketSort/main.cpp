#include <iostream>
#include <algorithm>
#include <windows.h>
#include <random>
#include "BucketSort.h"
#include <chrono>
#include "StreamManager.h"

int main() {
	/*
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint64_t> distribution(0);

	uint64_t* input = new uint64_t[1 << 16];
	uint64_t* output = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = distribution(gen);
	}
	BucketSort b(input, output, 1 << 16, 0);
	b.sort();
	b.printArray(output, 1 << 16);
	std::cout << b.isSorted();
	*/
	//flag stops after different levels eg L0 l1 to time benchmark
	//L0 with vortex S stream 5.3, append only can improve, corner cases
	// 
	//interval tree later
	//flag as parameter
	//static is ceiling 
	
	//put the timers here
	


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint64_t> distribution(0);

	uint64_t* input = new uint64_t[1 << 16];
	uint64_t* output = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = distribution(gen);
	}
	BucketSort b(input, output, 1 << 16, 0);
	b.sort();
	b.printArray(output, 1 << 16);
	std::cout << b.isSorted();
	StreamManager sm(10, 30);
}