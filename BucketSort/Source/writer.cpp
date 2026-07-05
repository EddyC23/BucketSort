#pragma once
#include "writer.h"

namespace datagen {
#pragma warning(push)
#pragma warning(disable: 4244)
	template<typename T>
	void Writer<T>::random_writer_mt(T* A, ui64 n, ui64 m, ui64 l) {
		if constexpr (IS_T_K32V32 || IS_T_K64V64) {
			std::mt19937_64 g(99999);
			std::uniform_int_distribution<ui64> d(l, m);

			std::mt19937_64 g2(11111);
			std::uniform_int_distribution<ui64> rnd;
			FOR(i, n, 1) {
				T kv;
				kv.key = d(g);
				kv.value = !random_values_ ? i : rnd(g2);
				A[i] = kv;
			}
		}
		else if constexpr IS_T_K64 {
			std::mt19937_64 g(99999);
			std::uniform_int_distribution<T> d(l, m);
			FOR(i, n, 1) A[i] = d(g);
		}
		else if constexpr IS_T_K32 {
			std::mt19937 g(99999);
			std::uniform_int_distribution<T> d(static_cast<T>(l), static_cast<T>(std::min(m, ui64(UINT32_MAX))));
			FOR(i, n, 1) A[i] = d(g);
		}		
		else
			ReportError("Type not supported");		

	}

	template<typename T>
	void Writer<T>::pareto_writer(T* A, ui64 n, WRITER_TYPE type) {
			ui64 a = 6364136223846793005, c = 1442695040888963407, x = 1;
			double ED = 20;
			double alpha = 1, beta = 7;
			ui64 sum = 0, Items = 0, y = 889;
			ui64 maxF = 0;

			// For random value generation
			std::mt19937_64 g;
			std::uniform_int_distribution<ui64> rnd;

			for (ui64 i = 0; i < n; ) {
				x = x * a + c;
				y = y * a + c;

				// Generate frequency from the Pareto distribution with alpha=1; otherwise, the generator gets slow
				double u = (double)y / ((double)(1LLU << 63) * 2);			// uniform [0,1]				
				
				// Rounded-up Pareto
				//ui64 f = std::min(ceil(beta * (1 / (1 - u) - 1)), double(10000));		
				
				// Unbounded pareto
				ui64 f = (ui64)(beta * (1 / (1 - u) - 1));
				if (rounded_pareto_) f = std::min(f, rounded_pareto_ceiling_);
				
				if (type == WRITER_TYPE::PARETO_B2B || type == WRITER_TYPE::PARETO_SHUFF) {
					if (i + f < n) {
						FOR(j, f, 1) {
							if constexpr IS_T_K32V32 {
								k32v32 kv;
								kv.key = x;
								kv.value = !random_values_ ? i + j : (ui)(rnd(g));
								A[i + j] = kv;
							}
							else if constexpr IS_T_K64V64 {
								k64v64 kv;
								kv.key = x;
								kv.value = !random_values_ ? i + j : rnd(g);
								A[i + j] = kv;
							}
							else A[i + j] = static_cast<T>(x);
						}
						i += f;
					}
					else if (i + 10 >= n) {
						for (; i < n; ++i) {
							if constexpr IS_T_K32V32 {
								k32v32 kv;
								kv.key = x;
								kv.value = !random_values_ ? i : (ui)(rnd(g));
								A[i] = kv;
							}
							else if constexpr IS_T_K64V64 {
								k64v64 kv;
								kv.key = x;
								kv.value = !random_values_ ? i : rnd(g);
								A[i] = kv;
							}
							else A[i] = static_cast<T>(x);
						}
					}
				}
				else if (type == WRITER_TYPE::PARETO_NONUNIFORM) {
					if constexpr IS_T_K32V32 {
						k32v32 kv;
						kv.key = f;
						kv.value = !random_values_ ? i : (ui)(rnd(g));
						A[i] = kv;
					}
					else if constexpr IS_T_K64V64 {
						k64v64 kv;
						kv.key = f;
						kv.value = !random_values_ ? i : rnd(g);
						A[i] = kv;
					}
					else A[i] = static_cast<T>(f);
					i++;
				}
			}

			if (type == WRITER_TYPE::PARETO_SHUFF) {
				std::random_device rd;
				std::mt19937_64 g(rd());
				std::shuffle(A, A + n, g);
			}
		}

	template<typename T>
	void Writer<T>::all_same(T* A, ui64 n) {
			std::mt19937_64 g;
			std::uniform_int_distribution<ui64> d;
			ui64 x = d(g);

			FOR(i, n, 1) {
				if constexpr IS_T_K32V32 {
					k32v32 kv;
					kv.key = x;
					kv.value = !random_values_ ? i : (ui)(d(g));
					A[i] = kv;
				}
				else if constexpr IS_T_K64V64 {
					k64v64 kv;
					kv.key = x;
					kv.value = !random_values_ ? i : d(g);
					A[i] = kv;
				}
				else A[i] = static_cast<T>(x);
			}
		}

	template<typename T>
	void Writer<T>::u_sequential(T* A, ui64 n) {
		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;
		FOR(i, n, 1) {
			if constexpr IS_T_K32V32 {
				k32v32 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : (ui)(rnd(g));
				A[i] = kv;
			}
			else if constexpr IS_T_K64V64 {
				k64v64 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : rnd(g);
				A[i] = kv;
			}
			else A[i] = static_cast<T>(i);
		}
	}

	// rSEQ: force all keys to go to same bucket by reducing the sequence to be contained only in the last KEY_BITS - 8 bits
	template<typename T>
	void Writer<T>::r_sequential(T* A, ui64 n) {
		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		T k;
		if constexpr (IS_T_K32 || IS_T_K64)
			k = 0;

		FOR(i, n, 1) {
			if constexpr IS_T_K32V32 {
				k32v32 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : (ui)(rnd(g));
				A[i] = kv;
			}
			else if constexpr IS_T_K64V64 {
				k64v64 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : rnd(g);
				A[i] = kv;
			}
			else A[i] = k | (i & (1LLU << ((sizeof(T) << 3) - 1)));
		}
	}

	template<typename T>
	void Writer<T>::sorted(T* A, ui64 n) {
		random_writer_mt(A, n);
		std::sort(std::execution::par_unseq, A, A + n);
	}

	template<typename T>
	void Writer<T>::reverse_sorted(T* A, ui64 n) {
		random_writer_mt(A, n);
		std::sort(std::execution::par_unseq, A, A + n, std::greater<>());
	}

	// random permutation of numbers from 0 to n - 1
	template<typename T>
	void Writer<T>::random_perm(T* A, ui64 n) {
		// For random value generation
		std::mt19937_64 g(99999);
		std::uniform_int_distribution<ui64> rnd;

		FOR(i, n, 1) {
			if constexpr IS_T_K32V32 {
				k32v32 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : (ui)(rnd(g));
				A[i] = kv;
			}
			else if constexpr IS_T_K64V64 {
				k64v64 kv;
				kv.key = i;
				kv.value = !random_values_ ? i : rnd(g);
				A[i] = kv;
			}
			else A[i] = static_cast<T>(i);
		}
		std::mt19937 g2(11111);
		std::shuffle(A, A + n, g2);
	}

	template<typename T>
	void Writer<T>::fibonacci(T* A, ui64 n) {
		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		ui64 a = 0, b = 1, c;
		if constexpr IS_T_K32V32 {
			k32v32 kv;
			kv.key = 0; kv.value = !random_values_ ? 0 : (ui)(rnd(g)); A[0] = kv;
			kv.key = 1; kv.value = !random_values_ ? 1 : (ui)(rnd(g)); A[1] = kv;
		}
		else if constexpr IS_T_K64V64 {
			k64v64 kv;
			kv.key = 0; kv.value = !random_values_ ? 0 : rnd(g); A[0] = kv;
			kv.key = 1; kv.value = !random_values_ ? 1 : rnd(g); A[1] = kv;
		}
		else {
			A[0] = 0; A[1] = 1;
		}

		ui64 i = 2;
		while (i < n) {
			c = a + b;
			if (c < b) {	// overflow
				a = 0; b = 1;
				if constexpr IS_T_K32V32 {
					k32v32 kv;
					kv.key = 0;
					kv.value = !random_values_ ? i : (ui)(rnd(g));
					A[i] = kv;
				}
				else if constexpr IS_T_K64V64 {
					k64v64 kv;
					kv.key = 0;
					kv.value = !random_values_ ? i : rnd(g);
					A[i] = kv;
				}
				else A[i] = 0;
				i++;
				if (i < n) {
					if constexpr IS_T_K32V32 {
						k32v32 kv;
						kv.key = 1;
						kv.value = !random_values_ ? i : (ui)(rnd(g));
						A[i] = kv;
					}
					else if constexpr IS_T_K64V64 {
						k64v64 kv;
						kv.key = 1;
						kv.value = !random_values_ ? i : rnd(g);
						A[i] = kv;
					}
					else A[i] = 1;
					i++;
				}
			}
			else {
				a = b;
				b = c;
				if constexpr IS_T_K32V32 {
					k32v32 kv;
					kv.key = b;
					kv.value = !random_values_ ? i : (ui)(rnd(g));
					A[i] = kv;
				}
				else if constexpr IS_T_K64V64 {
					k64v64 kv;
					kv.key = b;
					kv.value = !random_values_ ? i : rnd(g);
					A[i] = kv;
				}
				else A[i] = static_cast<T>(b);
				i++;
			}
		}
	}

	// set every 7 key of a sorted sequence to MAX
	template<typename T>
	void Writer<T>::almost_sorted(T* A, ui64 n) {		
		sorted(A, n);
		FOR(i, n, 7) {
			if constexpr IS_T_K32V32 A[i].key = UINT32_MAX;
			else if constexpr IS_T_K64 A[i] = UINT64_MAX;
			else if constexpr IS_T_K32 A[i] = UINT32_MAX;
			else if constexpr IS_T_K64V64 A[i].key = UINT64_MAX;				
		}
	}

	// 1/3rds of keys const, 2/3rds random; then shuffled
	template<typename T>
	void Writer<T>::const_random(T* A, ui64 n) {
		std::mt19937_64 gen;
		std::uniform_int_distribution<ui64> dis(0, ~0ull);

		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		// write 2/3rds random keys
		ui64 i = 0;
		for (; i < 2 * n / 3; ++i) {
			ui64 x = dis(gen);
			if constexpr (IS_T_K32V32 || IS_T_K64V64) {				
				T kv;
				kv.key = x; 
				kv.value = !random_values_ ? i : rnd(g); 
				A[i] = kv;
			}
			else A[i] = static_cast<T>(x);
		}

		// write 1/3rds const keys
		ui64 x = dis(gen);
		for (; i < n; ++i) {
			if constexpr (IS_T_K32V32 || IS_T_K64V64) {
				T kv;
				kv.key = x; 
				kv.value = !random_values_ ? i : rnd(g); 
				A[i] = kv;
			}
			else A[i] = static_cast<T>(x);
		}
		std::random_device rd;
		std::mt19937_64 g2(rd());
		std::shuffle(A, A + n, g2);
	}

	// normal distribution w/ miu = max/2 and sigma = max/6
	template<typename T>
	void Writer<T>::normal(T* A, ui64 n, ui64 miu, ui64 sigma) {
#pragma warning(push)
#pragma warning(disable: 4244)

		// For random value generation
		std::mt19937_64 g(99999);
		std::uniform_int_distribution<ui64> rnd;

		if constexpr IS_T_K32 {
			miu = UINT32_MAX >> 1;
			sigma = (UINT32_MAX >> 1) / 3;
			std::mt19937 gen(11111);
			std::normal_distribution<> dis(miu, sigma);

			FOR(i, n, 1)
				A[i] = std::round(dis(gen));
		}
		else if constexpr IS_T_K32V32 {
			miu = UINT32_MAX >> 1;
			sigma = (UINT32_MAX >> 1) / 3;
			std::mt19937 gen(11111);
			std::normal_distribution<> dis(miu, sigma);

			FOR(i, n, 1) {
				k32v32 kv;
				kv.key = std::round(dis(gen));
				kv.value = !random_values_ ? i : rnd(g);
				A[i] = kv;
			}
		}
		else if constexpr IS_T_K64 {
			miu = UINT64_MAX >> 1;
			sigma = (UINT64_MAX >> 1) / 3;
			std::mt19937_64 gen(11111);
			std::normal_distribution<> dis(miu, sigma);

			FOR(i, n, 1)
				A[i] = std::round(dis(gen));
		}
			
		else if constexpr IS_T_K64V64 {
			miu = UINT64_MAX >> 1;
			sigma = (UINT64_MAX >> 1) / 3;
			std::mt19937_64 gen(11111);
			std::normal_distribution<> dis(miu, sigma);

			FOR(i, n, 1) {
				k64v64 kv;
				kv.key = std::round(dis(gen));
				kv.value = !random_values_ ? i : rnd(g); 
				A[i] = kv;
			}
		}
		else
			ReportError("Type not supported");
#pragma warning(pop)
	}

	template<typename T>
	void Writer<T>::crand_gaussian(T* A, ui64 n) {
#pragma warning(push)
#pragma warning(disable: 4244)
		std::mt19937_64 gen;
		std::uniform_int_distribution<ui64> dis(0, ~0ull);

		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		FOR(i, n, 1) {
			ui64 a = dis(gen); ui64 b = dis(gen); ui64 c = dis(gen); ui64 d = dis(gen);
			ui64 x = (a >> 2) + (b >> 2) + (c >> 2) + (d >> 2) + (a & 4 + b & 4 + c & 4 + d & 4) / 4;
			if constexpr (IS_T_K32V32 || IS_T_K64V64) {
				T kv;
				kv.key = x; kv.value = !random_values_ ? i : rnd(g); A[i] = kv;
			}
			else
				A[i] = x;
		}
#pragma warning(pop)
	}

	template<typename T>
	void Writer<T>::uniform_double(T* A, ui64 n) {
		// For random value generation
		std::mt19937_64 g(99999);
		std::uniform_int_distribution<ui64> rnd;

		if constexpr IS_T_K32 {
			float* p = (float*)A;
			std::uniform_real_distribution<float> dis(0.0, FLT_MAX);
			std::mt19937 gen(11111);
			FOR(i, n, 1)
				p[i] = dis(gen);
		}
		else if constexpr IS_T_K32V32 {
			std::uniform_real_distribution<float> dis(0.0, FLT_MAX);
			std::mt19937 gen(11111);
			float* p = (float*)A;
			FOR(i, n, 1) {				
				// k32v32 has (value, key) sequence
				ui* k = (ui*)(p);
				*k = random_values_ ? rnd(g) : i;
				p++;
				*p++ = dis(gen);				
			}
		}
		else if constexpr IS_T_K64 {
			double* p = (double*)A;
			std::uniform_real_distribution<double> dis(0.0, DBL_MAX);
			std::mt19937_64 gen(11111);
			FOR(i, n, 1)
				p[i] = dis(gen);
		}
			
		else if constexpr IS_T_K64V64 {
			std::uniform_real_distribution<double> dis(0.0, DBL_MAX);
			std::mt19937_64 gen(11111);
			double* p = (double*)A;
			FOR(i, n, 1) {
				*p = dis(gen);
				ui64* k = (ui64*)(p + 1);
				*k = random_values_ ? rnd(g) : i;
				p += 2;
			}
		}

	}

	// NOTE: worst case inputs not updated for key-value pairs
	template<typename T>
	void Writer<T>::worst_case_msd(T* A, ui64 n) {
#pragma warning(push)
#pragma warning(disable: 4244)
		/*
		the idea is to keep the uniform distribution on first two levels 8+8 bits
		after that, all items should go to the same bucket but the Items should not be
		the same; so the last few bits should vary
		Q. should we enforce SN30/31 or make the group large enough to do another level of split?
		*/
		if constexpr (IS_T_K64 || IS_T_K32) {
			std::mt19937_64 gen(99999);
			std::uniform_int_distribution<ui64> dis(0, UINT64_MAX);
			std::mt19937_64 gen2(11111);
			std::uniform_int_distribution<ui64> dis2(0, 255);
			const ui base_sort = 32;

			T* p = A;
			ui steps = sizeof(T);					// if 8 bytes -> 5 steps; 4 bytes -> 1 step
			ui shift_begin = (sizeof(T) << 3);		// first two levels keep random

			ui64 i = 0;
			for (; i <= n - (base_sort + steps); i += (base_sort + steps)) {
				ui64 x = dis(gen);
				int k = 0;
				for (; k < base_sort; ++k) {							// base_sort keys going to same buckets
					p[i + k] = x + k;
				}
				for (int j = shift_begin; j >= 8; j -= 8) {
					ui64 mask = 255LLU << j;
					ui64 x1 = (x & ~mask) | ((x & mask) ^ mask);		// add a key per level to prohibit all keys going to same bucket
					if (i + k >= n) return;
					p[i + k] = x1;
					k++;
				}
			}
			while (i < n) {
				ui64 x = dis(gen);
				p[i] = x;
				i++;
			}
		}
		else ReportError("Type not supported");
#pragma warning(pop)
	}

	template<typename T>
	void Writer<T>::worst_case_msd_2(T* A, ui64 n) {
		if constexpr IS_T_K64 {
			std::mt19937_64 gen;
			std::uniform_int_distribution<ui64> dis(0, UINT64_MAX);

			std::mt19937_64 gen2;
			std::uniform_int_distribution<ui64> dis2(0, 255);

			for (ui64 i = 0; i <= n - 37; i += 37) {
				ui64 x = dis(gen);
				for (int j = 40; j >= 8; j -= 8) {
					ui64 mask = 255LLU << j;
					ui64 y = dis2(gen2);
					x = (x & ~mask) | (y << j);
				}
				int k = 0;
				for (; k < 32; ++k)
					A[i + k] = x + k;
				for (int j = 40; j >= 8; j -= 8) {
					ui64 mask = 255LLU << j;
					ui64 x1 = (x & ~mask) | ((x & mask) ^ mask);
					A[i + k++] = x1;
				}
			}
		}
		else ReportError("Type not supported");
	}

	template<typename T>
	void Writer<T>::worst_case_bscan(T* A, ui64 n) {
		if constexpr (IS_T_K32 || IS_T_K64) {
			const ui shift = 8;
			ui size_bytes = sizeof(T);
			ui64 mx = (1LLU << (size_bytes << 3)) - 1;

			ui64 i = 0;
			//FFFFFFF.., 00FFFFFF.., 0000FFFF...
			while (mx) {
				A[i++] = static_cast<T>(mx);
				mx >>= shift;
			}
			// fill the rest with zeros
			//memset(A + i, 0, (n - i) * size_bytes);
			while (i < n) A[i] = static_cast<T>(i++);
		}
	}


	template<typename T>
	void Writer<T>::worst_case_fscan(T* A, ui64 n) {
		if constexpr (IS_T_K32 || IS_T_K64) {					
			ui key_size_bytes = sizeof(T);
			ui64 mx = (1LLU << (key_size_bytes << 3)) - 1;
			memset(A, 0xFF, n * key_size_bytes);
			ui64 prev = 0;
			FOR_INIT(i, 1, key_size_bytes + 1, 1) {
				A[n - i] = static_cast<T>(prev);
				prev += 0xFFllu << ((8-i) << 3);
			}
		}
	}


	template<typename T>
	void Writer<T>::qsort_worst_case(T* A, ui64 n) {
		if constexpr IS_T_K32 {
			ui aLCG = 214013, cLCG = 2531011, x = 289;
			/*ui k = INT32_MAX;
			for (int i = 0; i < n; i++) {
				x = x * aLCG + cLCG;
				double u = (double)x / UINT_MAX;
				if (u < 0.92)
					A[i] = n;
				else
					A[i] = i;
			}*/
			FOR(i, n, 1) {
				x = x * aLCG + cLCG;
				double u = (double)x / UINT_MAX;
				if (u < 0.92) A[i] = static_cast<T>(n);
				else if (u < 0.94) A[i] = static_cast<T>(i * i);
				else A[i] = static_cast<T>(n - i);

				//else A[i] = i;
				//else A[i] = k--;
			}

			//ui64 i = 0;
			//A[i++] = 0;
			//k = INT32_MAX >> 1;
			//while (k < 0x7ffffffe) {
			//	A[i++] = k - 1;
			//	A[i++] = k;
			//	k = k + ((INT32_MAX - k) >> 1);
			//}
			//while (i < n) A[i++] = INT32_MAX - 1; // n;
			//FOR(i, 100, 1) lgr.write("%d ", A[i]); lgr.write("\n");

			// failed attempt
			/*
			// get the random indices for median computation
			int indices[] = {
				766, 607, 926, 742, 194, 853, 306, 713, 366, 780, 658, 858, 899, 472, 71, 172, 998, 450, 676, 401,
				144, 352, 16, 521, 672, 700, 259, 311, 551, 1009, 322, 792, 651, 208, 952, 688, 395, 491, 207, 255,
				712, 375, 748, 789, 609, 113, 609, 437, 934, 565, 536, 151, 510, 540, 100, 285, 290, 861, 545, 386,
				46, 931, 225, 503, 484, 259, 88, 146, 546, 377, 974, 797 };

			ui n_indices = 72;
			std::sort(std::execution::par_unseq, indices, indices + n_indices);

			// remove dups
			int prev = indices[0];
			ui dups = 0;
			FOR_INIT(i, 1, n_indices, 1) {
				if (indices[i] == prev) {
					dups++;
					indices[i] = INT32_MAX;
				}
				else {
					prev = indices[i];
				}
			}
			std::sort(std::execution::par_unseq, indices, indices + n_indices);
			n_indices = n_indices - dups;

			int i = n - 1;
			int j = n_indices - 1;
			int k = n;
			int mx = 1LU << 29;
			while (i >= 0) {
				if (j >= 0 && i == indices[j]) {
					A[i] = 1LU << 30;
					j--;
				}
				else if (mx > 0) {
					A[i] = mx;
					mx >>= 1;
				}
				else A[i] = k--;
				--i;
			}
			A[0] = 0;*/
		}
		else if constexpr IS_T_K64 {
			/*std::mt19937_64 g;
			std::uniform_int_distribution<ui64> d;*/
			uint64_t a = 6364136223846793005;
			uint64_t c = 1442695040888963407;
			ui64 x = 1e4;
			FOR(i, n, 1) {
				//ui64 x = d(g);
				x = x * a + c;
				double u = (double)x / UINT64_MAX;
				if (u < 0.92) A[i] = n;
				else if (u < 0.94) A[i] = i * i;
				else A[i] = n - i;

			}
		}
		else ReportError("Type not supported");
	}
	
	template<typename T>
	void Writer<T>::predictable_dups(T* A, ui64 n, ui64 rep) {
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		ui64 a = 6364136223846793005, c = 1442695040888963407, x = 1;
		uint64_t i = 0;
		for (; i <= n - rep; i += rep) {
			x = x * a + c;
			FOR(j, rep, 1) {
				if constexpr (IS_T_K32V32 || IS_T_K64V64) {
					T kv;
					kv.key = x; kv.value = !random_values_ ? i + j : rnd(g); A[i + j] = kv;
				}
				else A[i + j] = x;
			}
		}
		for (; i < n; ++i) {
			x = x * a + c;
			if constexpr (IS_T_K32V32 || IS_T_K64V64) {
				T kv;
				kv.key = x; kv.value = !random_values_ ? i : rnd(g); A[i] = kv;
			}
			else A[i] = x;
		}
	}

	template<typename T>
	void Writer<T>::unpredictable_dups(T* A, ui64 n, ui64 X1, ui64 X2) {
		// For random value generation
		std::mt19937_64 g;
		std::uniform_int_distribution<ui64> rnd;

		srand(time(NULL));
		ui64 a = 6364136223846793005, c = 1442695040888963407, x = 1;
		uint64_t i = 0;
		for (; i < n;) {
			double r = rand() * 1.0 / RAND_MAX;
			x = x * a + c;
			if (r > 0.5 && i + X1 < n) {
				FOR(j, X1, 1) {
					if constexpr (IS_T_K32V32 || IS_T_K64V64) {
						T kv;
						kv.key = x; kv.value = !random_values_ ? i + j : rnd(g); A[i + j] = kv;
					}
					else A[i + j] = x;
				}
				i += X1;
			}
			else if (i + X2 < n) {
				FOR(j, X2, 1) {
					if constexpr (IS_T_K32V32 || IS_T_K64V64) {
						T kv;
						kv.key = x; kv.value = !random_values_ ? i + j : rnd(g); A[i + j] = kv;
					}
					else A[i + j] = x;
				}
				i += X2;
			}
			else {
				for (; i < n; ++i) {
					if constexpr (IS_T_K32V32 || IS_T_K64V64) {
						T kv;
						kv.key = x; kv.value = !random_values_ ? i : rnd(g); A[i] = kv;
					}
					else A[i] = x;
				}
			}
		}
	}
	
	template<typename T>
	void Writer<T>::write_combine_adversarial(T* A, ui64 n, ui64 LSD) {
		if constexpr (IS_T_K32 || IS_T_K64) {
			int bits = sizeof(T) << 3;
			int skip = 16;
			int nBuckets = 256;
			int d = nBuckets / skip;
			FOR(i, n, 1) {
				ui64 x = 0, R = i;
				FOR(j, bits, 8) {
					x += ((R % d) * skip) << j;
					R /= d;
				}
				if constexpr IS_T_K64 {
					A[i] = (x << 32) | (x & (~0LLU >> 32));
					if (LSD == false)
						A[i] = flip_bytes64(A[i]);
				}
				else {
					A[i] = x;
					if (LSD == false)
						A[i] = flip_bytes32(A[i]);
				}
				//lgr.write("%20llX\n", A[i]);
			}
		}
		else {
			ReportError("Type not implemented");
		}
	}

	template<typename T>
	void Writer<T>::mid_zeros(T* A, ui64 n) {
		// For random values
		std::mt19937_64 g(99999);
		std::uniform_int_distribution<ui64> rnd;
		// key only
		if constexpr IS_T_K32V32 {
			std::mt19937 g2(11111);
			std::uniform_int_distribution<ui> d;
			FOR(i, n, 1) {
				k32v32 kv;
				kv.key = d(g2) & 0xFE00007F;
				kv.value = !random_values_ ? i : (ui)(rnd(g));		
				A[i] = kv;
			}
		}
		else if constexpr IS_T_K64V64 {
			std::mt19937_64 g2(11111);
			std::uniform_int_distribution<ui64> d;
			FOR(i, n, 1) {
				k64v64 kv;
				kv.key = d(g2) & 0xFE0000000000007F;
				kv.value = !random_values_ ? i : rnd(g);
				A[i] = kv;
			}
		}
		else if constexpr IS_T_K64 {
			std::mt19937_64 g2(11111);
			std::uniform_int_distribution<T> d;
			FOR(i, n, 1) A[i] = d(g2) & 0xFE0000000000007F;
		}
		else if constexpr IS_T_K32 {
			std::mt19937 g2(11111);
			std::uniform_int_distribution<T> d;
			FOR(i, n, 1) A[i] = d(g2) & 0xFE00007F;
		}
		else
			ReportError("Type not supported");
	}

	template<typename T>
	void Writer<T>::usage() {
		printf("> Input types:\n");
		for (int i = 1; i < writer_count_; ++i) {
			printf("%2d %s\n", i, writer_names_[i]);
			if (i == MT) {
				for (int j = 0; j < uniform_ranges_count_; ++j) {
					printf("    %2d %s\n", j, uniform_ranges_[j]);
				}
			}
		}
	}


	// l, m is the range for MT random Items only
	template<typename T>
	char* Writer<T>::generate(T* A, ui64& n, WRITER_TYPE type, ui64 m, ui64 l, ui64 rep, ui64 X1, ui64 X2) {
		int key_bits = -1, value_bits = -1;
		if constexpr (IS_T_K32V32) {
			key_bits = value_bits = 32;
		}
		else if constexpr (IS_T_K64V64) {
			key_bits = value_bits = 64;
		}
		else { 
			key_bits = sizeof(T) << 3; value_bits = 0; 
		}
		log_offset_ = 0;
		int written = std::snprintf(
			log_ + log_offset_,
			sizeof(log_) - log_offset_,
			"Writer: Generating %s keys of total %llu bytes (key: %d bits, value: %d bits, rounded-pareto: %s, random-values: %s) ... \n",
			writer_names_[type],
			n * sizeof(T),
			key_bits,
			value_bits,
			rounded_pareto_ ? "true" : "false",
			random_values_ ? "true" : "false"
		);
		if (written > 0) log_offset_ += written;

		if (type == MT)
			random_writer_mt(A, n, m, l);
		else if (type == RANDOM_PERM_N)
			random_perm(A, n);
		else if (type == SORTED)
			sorted(A, n);
		else if (type == REV_SORTED)
			reverse_sorted(A, n);
		else if (type == FIB)
			fibonacci(A, n);
		else if (type == ALMOST_SORTED)
			almost_sorted(A, n);
		else if (type == CONST_RAND)
			const_random(A, n);
		else if (type == NORMAL)
			normal(A, n);
		else if (type == CRAND_GAUSSIAN)
			crand_gaussian(A, n);
		else if (type == UNIFORM_DBL)
			uniform_double(A, n);
		else if (type == WORST_CASE)
			worst_case_msd(A, n);
		else if (type == WORST_CASE2)
			worst_case_msd_2(A, n);
		else if (type == ALL_SAME)
			all_same(A, n);
		else if (type == U_SEQ)
			u_sequential(A, n);
		else if (type == R_SEQ)
			r_sequential(A, n);
		else if (type == PARETO_SHUFF || type == PARETO_B2B || type == PARETO_NONUNIFORM)
			pareto_writer(A, n, type);
		else if (type == PD)
			predictable_dups(A, n, rep);
		else if (type == UD)
			unpredictable_dups(A, n, X1, X2);
		else if (type == WORST_CASE_BACKSCAN)
			worst_case_bscan(A, n);
		else if (type == WORST_CASE_FSCAN)
			worst_case_fscan(A, n);
		else if (type == WORST_CASE_QSORT)
			qsort_worst_case(A, n);
		else if (type == WC_ADVERSARIAL_LSD)
			write_combine_adversarial(A, n, true);
		else if (type == WC_ADVERSARIAL_MSD)
			write_combine_adversarial(A, n, false);
		else if (type == PD16)
			predictable_dups(A, n, 16);
		else if (type == PD64)
			predictable_dups(A, n, 64);
		else if (type == PD512)
			predictable_dups(A, n, 512);
		else if (type == MID_ZEROS)
			mid_zeros(A, n);

		written = std::snprintf(
			log_ + log_offset_,
			sizeof(log_) - log_offset_,
			"Writer: Done.\n"
		);
		if (written > 0) log_offset_ += written;

		return log_;
	}

	template class datagen::Writer<ui>;
	template class datagen::Writer<ui64>;
	template class datagen::Writer<k32v32>;
	template class datagen::Writer<k64v64>;
#pragma warning(pop)
};	// end namespace
