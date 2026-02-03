#include <iostream>
#include <windows.h>
#include <algorithm>
const int D = 8;
const int K = 1 << 8;
uint64_t* bucket[D][K];
uint64_t* output = (uint64_t*)malloc(sizeof(uint64_t) * 512);
// depth of 8 64 bits with 8 bits looked at per bucket
// 1 << 8 2 ^ 8 buckets 

void bucketSort(uint64_t * buf, uint64_t size, int shift, int level) {
	
	uint64_t ** p = &bucket[level][0];
	uint64_t** pNext = p + K;
	memcpy(pNext, p, sizeof(uint64_t*) * K);
	for (uint64_t i = 0; i < size; i++) {
		
		uint64_t mask = (1 << 8) - 1;
		uint64_t idx = (buf[i] >> shift) & mask;
		//std::cout << buf[i] << " " << idx << "\n";
		std::cout << "Level" << level << " ";
		std::cout << "Index" << idx << "\n";
		*pNext[idx]++ = buf[i];
	}
	for (uint64_t j = 0; j < K; j++) {
		uint64_t sizeNext = pNext[j] - p[j];
		if (sizeNext > 32) {
			bucketSort(p[j], sizeNext, shift - 8, level + 1);
		}
		else {
			std::sort(p[j], p[j] + sizeNext);
			memcpy(output, p[j], sizeof(uint64_t) * sizeNext);
			output += sizeNext;
		}

	}

}
void printArray(uint64_t* ptr, uint64_t size) {
	for (size_t i = 0; i < size; i++) {
		std::cout << ptr[i] << "\n";
	}
}
int main() {
	uint64_t* input = new uint64_t[1 << 16];
	for (size_t i = 0; i < 1 << 16; i++) {
		input[i] = (1 << 16) - i;
	}
	for (int i = 0; i < D; i++) {
		for (int j = 0; j < K; j++) {
			bucket[i][j] = new uint64_t[1 << 16];
		}
	}
	//printArray(input, 1 << 16);
	bucketSort(input, 1 << 16, 64 - 8, 0);

	printArray(input, 1 << 16);
	
}