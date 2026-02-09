#include <iostream>
#include <algorithm>
#include <windows.h>
#include <BucketSort.h>
void printArray(uint64_t* ptr, uint64_t size) {
	for (size_t i = 0; i < size; i++) {
		std::cout << "index" << i << " : " << ptr[i] << "\n";
	}
}
// create a base casejj
const int D = 8;
const int K = 1 << 8;
uint64_t* bucket[D + 1][K];
uint64_t* output = new uint64_t[1 << 16];
uint64_t* base = output;
// depth of 8 64 bits with 8 bits looked at per bucket
// 1 << 8 2 ^ 8 buckets 

void bucketSort(uint64_t * buf, uint64_t size, int shift, int level) {
	//printArray(buf, size);
	uint64_t ** p = &bucket[level][0];
	std::cout << level << " " << shift << "\n";
	if (level ==  8) {
		
		for (uint64_t j = 0; j < K; j++) {
			uint64_t sizeNext = p[j] - bucket[7][j];
			std::cout << "LEVEL 8 : " << j<<  "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
			printArray(p[j], sizeNext);
			memcpy(output, bucket[7][j], sizeof(uint64_t) * sizeNext);
			output += sizeNext;
		}
		return;

	}
	//pointer to the first bucket pointer for the current level of recursion where to write
	uint64_t** pNext = p + K;
	//pointer to the first bucket pointer for the next level of recursion
	memcpy(pNext, p, sizeof(uint64_t*) * K);
	//copy the pointers from the current level pointer to the first bucket pointer for the next level of recursion so we can append and calulate size from it

	
	for (uint64_t i = 0; i < size; i++) {
		//iterate through the size of how many you need to sort for the current bucket (the ones from the previous level of recursion)
		//size is the lenght of the buff that you should be soirting
		uint64_t mask = (1 << 8) - 1;
		uint64_t idx = (buf[i] >> shift) & mask;
		//shift and mask
		
		*pNext[idx]++ = buf[i];
		//increment the pointer for the bucket that matches the key and writing the current element to the original value of the pointer
	}
	for (uint64_t j = 0; j < K; j++) {
		//the size for the next level of recursion for the current bucket is this
		uint64_t sizeNext = pNext[j] - p[j];
		
		if (sizeNext > 32) {
			bucketSort(p[j], sizeNext, shift - 8, level + 1);

		}
		else {
			std::sort(p[j], p[j] + sizeNext);
			//std::cout << "After Calling STD SORT THIS WILL BE APPENDED" << "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" << "\n";
			//printArray(p[j], sizeNext);
			memcpy(output, p[j], sizeof(uint64_t) * sizeNext);
			output += sizeNext;
		}

	}

}

int main() {
	uint64_t* input = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = (1 << 16) - i;
		
	}
	
	for (int j = 0; j < K; j++) {
		bucket[0][j] = new uint64_t[D * (1 << 16)];
	}
	

	//printArray(input, 1 << 16);
	
	bucketSort(input, 1 << 16, 64 - 8, 0);
	
//	printArray(base, 1 << 16);

	
	
}