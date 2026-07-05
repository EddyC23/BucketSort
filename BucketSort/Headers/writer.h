#pragma once
#include "commons.h"
#include <fstream>
#include <unordered_set>
#include <execution>

namespace datagen {
	typedef enum {
		MT,
		ALL_SAME,
		SORTED,
		REV_SORTED,
		ALMOST_SORTED,
		PARETO_NONUNIFORM,
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
		PD,
		UD,
		U_SEQ,
		R_SEQ,
		WORST_CASE_BACKSCAN,
		WORST_CASE_FSCAN,
		WORST_CASE_QSORT,
		WC_ADVERSARIAL_MSD,
		WC_ADVERSARIAL_LSD,
		PD16,
		PD64,
		PD512,
	} WRITER_TYPE;

#define UNIFORM_64			0
#define UNIFORM_32			1
#define UNIFORM_N_BY_4		2
#define UNIFORM_N			3
#define UNIFORM_3N			4
#define UNIFORM_10N			5
#define UNIFORM_UPPER_N		6
#define UNIFORM_CUSTOM		7

	template <typename T>
	class Writer {
		const bool rounded_pareto_ = true;
		const ui64 rounded_pareto_ceiling_ = 10000;
		const bool random_values_ = true;
		char log_[2048];
		int log_offset_ = 0;

		void random_writer_mt(T* A, ui64 n, ui64 m = ~0ull, ui64 l = 0);
		void pareto_writer(T* A, ui64 n, WRITER_TYPE type);
		void all_same(T* A, ui64 n);
		void u_sequential(T* A, ui64 n);
		// rSEQ: force all keys to go to same bucket by reducing the sequence to be contained only in the last KEY_BITS - 8 bits
		void r_sequential(T* A, ui64 n);
		void sorted(T* A, ui64 n);
		void reverse_sorted(T* A, ui64 n);
		// random permutation of numbers from 0 to n - 1
		void random_perm(T* A, ui64 n);
		void fibonacci(T* A, ui64 n);
		// set every 7 key of a sorted sequence to MAX
		void almost_sorted(T* A, ui64 n);
		// 1/3rds of keys const, 2/3rds random; then shuffled
		void const_random(T* A, ui64 n);
		// normal distribution w/ miu = max/2 and sigma = max/6
		void normal(T* A, ui64 n, ui64 miu = (UINT64_MAX >> 1), ui64 sigma = (UINT64_MAX >> 1) / 3);
		void crand_gaussian(T* A, ui64 n);
		void uniform_double(T* A, ui64 n);
		// NOTE: worst case inputs not updated for key-value pairs
		void worst_case_msd(T* A, ui64 n);
		void worst_case_msd_2(T* A, ui64 n);
		void worst_case_bscan(T* A, ui64 n);
		void worst_case_fscan(T* A, ui64 n);
		void qsort_worst_case(T* A, ui64 n);
		void predictable_dups(T* A, ui64 n, ui64 rep);
		void unpredictable_dups(T* A, ui64 n, ui64 X1, ui64 X2);
		void write_combine_adversarial(T* A, ui64 n, ui64 LSD = true);	// flip bytes if LSD = false
		void mid_zeros(T* A, ui64 n);

		ui64 flip_bytes64(ui64 x) {
			return ((x >> 56) & 0x00000000000000FFULL) |
				((x >> 40) & 0x000000000000FF00ULL) |
				((x >> 24) & 0x0000000000FF0000ULL) |
				((x >> 8) & 0x00000000FF000000ULL) |
				((x << 8) & 0x000000FF00000000ULL) |
				((x << 24) & 0x0000FF0000000000ULL) |
				((x << 40) & 0x00FF000000000000ULL) |
				((x << 56) & 0xFF00000000000000ULL);
		}

		ui flip_bytes32(ui x) {
			return ((x >> 24) & 0x000000FF) |
				((x >> 8) & 0x0000FF00) |
				((x << 8) & 0x00FF0000) |
				((x << 24) & 0xFF000000);
		}

		/**
		* @brief Flips the key and value contents of a key-value struct.
		* Main use of this function is to convert k32v32 (which contains 4-byte key then 4-byte value) into a 64-bit int.
		*
		* @param kv Reference to key value object.
		*
		* @return Swap contents of key-value in-place.
		*/
		void flip_kv(T& kv) {
			if constexpr (IS_T_K32V32 || IS_T_K64V64)
				std::swap(kv.key, kv.value);
		}
	public:
		static inline const char* uniform_ranges_[] = { "2^64 - 1", "2^32 - 1", "n/4", "n-1", "3n-1", "10n-1", "[MAX - n + 1, MAX]", "Custom [l, m]" };
		static const int uniform_ranges_count_ = sizeof(uniform_ranges_) / sizeof(char*);

		static inline const char* writer_names_[] = { 
			"Mersenne-Twister", "All-Same", "Sorted-sequence", "Reverse-sorted", "Almost-sorted", "Pareto-keys", "Pareto-b2b", "Pareto-shuffled",
			"Fibonacci-sequence", "Normal", "Uniform-double","Mid-Zeros",
			"Random-permutation", "Constant-and-random","crand-Gaussian", "Worst-case", "Worst-case-2",
			"Predictable-dups", "Unpredictable-dups", "u-Sequential", "r-Sequential", "Worst-case-bscan", "Worst-case-fscan", "Worst-case-qsort", "WC-adversarial-MSD", "WC-adversarial-LSD",
			"PD16", "PD64", "PD512"

		};
		static const int writer_count_ = sizeof(writer_names_) / sizeof(char*);

		void usage();
		// l, m is the range for MT random Ts only
		char* generate(T* A, ui64& n, WRITER_TYPE type, ui64 m = ~0ull, ui64 l = 0, ui64 rep = 1, ui64 X1 = 1, ui64 X2 = 1);

		/**
		* @brief Returns the log string that gets updated by internal functions. 
		* Avoid printing anything from within datagen and make log available to caller for debugging.
		* 
		* @return Log string.
		*/
		char* get_log() { return log_; }
	};
};
