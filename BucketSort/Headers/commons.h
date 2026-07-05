#pragma once
#pragma comment(lib, "Advapi32.lib")
#define NOMINMAX
#include <Windows.h>
#include <random>
#include <chrono>
#include <mmintrin.h>
#include <intrin.h>		// MMX
#include <emmintrin.h>	// SSE
#include <immintrin.h>	// AVX
//#include <zmmintrin.h>	// AVX-512
#include <stdio.h>
#include <filesystem>
#define MAX_PRINTOUT	1024
#define PRINT(fmt, ...) { char buf_PRINT[MAX_PRINTOUT] = "%s: "; strcat_s(buf_PRINT, MAX_PRINTOUT, fmt); printf (buf_PRINT, __FUNCTION__, ##__VA_ARGS__); }
#define ReportError(fmt, ...) { PRINT(fmt, ##__VA_ARGS__); \
								exit(-1); }
typedef unsigned __int64 uint64;
typedef __int64 int64;

using namespace std;
using namespace std::chrono;
using hrc = high_resolution_clock;

// typedef Item
typedef int64_t  	i64;
typedef uint64_t 	ui64;
typedef uint32_t 	ui;
typedef uint8_t		uchar;
typedef uint16_t 	ushort;

// typedef simd stuff
typedef __m128i sse;
typedef __m128	ssef;
typedef __m128d	ssed;
typedef __m256i avx2;
typedef __m256	avx2f;
typedef __m256d avx2d;
typedef __m512i avx512;
typedef __m512	avx512f;
typedef __m512d avx512d;

// NOTE: for 32+32 bit KV, use Key32Value32 below
#pragma pack(push, 1)
template <typename Keytype, typename Valuetype>
struct KeyValue {
	Keytype key;
	Valuetype value;
	// NOTE: operator overloads not used in origami for KV pair as it is slow; used mainly for correctness checking with std::sort 
	bool operator <(const KeyValue& kv) const {
		return key < kv.key;
	}
	bool operator >(const KeyValue& kv) const {
		return key > kv.key;
	}
	bool operator !=(const KeyValue& kv) const {
		return key != kv.key;
	}
	bool operator <=(const KeyValue& kv) const {
		return key <= kv.key;
	}
	KeyValue& operator>>(ui64 scalar) {
		key >>= scalar;
		return *this;
	}
	KeyValue& operator&(ui64 scalar) {
		key &= scalar;
		return *this;
	}
	ui64 operator*(ui64 scalar) const {
		return scalar * key;
	}
	KeyValue& operator|(ui64 scalar) {
		key |= scalar;
		return *this;
	}
	bool operator==(const KeyValue& other) const {
		return key == other.key;
	}
	void print() {
		if constexpr (std::is_same<Keytype, ui64>::value && std::is_same<Valuetype, ui64>::value) {
			printf("%20llX %20llX\n", key, value);
		}
		else {
			ReportError("print not implemented for this key-value combination");
		}
	}
};
#pragma pack(pop)
template struct KeyValue<i64, i64>;
template struct KeyValue<ui64, ui64>;
typedef KeyValue<ui64, ui64> k64v64;

// Special case for 32+32 to support 64-bit sorts by making the struct big-endian
#pragma pack(push, 1)
template<typename K, typename V>
struct Key32Value32 {
	V value;
	K key;
	// NOTE: operator overloads not used in origami for KV pair as it is slow; used mainly for correctness checking with std::sort 
	bool operator <(const Key32Value32& kv) const {
		return key < kv.key;
	}
	bool operator >(const Key32Value32& kv) const {
		return key > kv.key;
	}
	bool operator !=(const Key32Value32& kv) const {
		return key != kv.key;
	}
	bool operator <=(const Key32Value32& kv) const {
		return key <= kv.key;
	}
	Key32Value32& operator>>(ui scalar) {
		key >>= scalar;
		return *this;
	}
	Key32Value32& operator&(ui64 scalar) {
		key &= scalar;
		return *this;
	}
	ui operator*(ui scalar) const {
		return scalar * key;
	}
	Key32Value32& operator|(ui64 scalar) {
		key |= scalar;
		return *this;
	}
	void print() {
		if constexpr (std::is_same<K, ui>::value && std::is_same<V, ui>::value) {
			printf("%20X %20X\n", key, value);
		}
		else {
			ReportError("print not implemented for this key-value combination");
		}
	}
};
#pragma pack(pop)
template struct Key32Value32<ui, ui>;
typedef Key32Value32<ui,ui> k32v32;

#define MIN(x, y)				((x)<(y)?(x):(y))
#define MAX(x, y)				((x)<(y)?(y):(x)) 
#define FOR(i,n,k)				for (ui64 (i) = 0; (i) < (n); (i)+=(k)) 
#define FOR_INIT(i, init, n, k)	for (ui64 (i) = (init); (i) < (n); (i) += (k)) 
#define PRINT_ARR(arr, n)		{ FOR((i), (n), 1) lgr.write("%lX ", (arr)[(i)]); lgr.write("\n"); }
#define PRINT_ARR64(arr, n)		{ FOR((i), (n), 1) lgr.write("%llX ", ((ui64*)arr)[(i)]); lgr.write("\n"); }
#define PRINT_DASH(n)			{ FOR(i, (n), 1) lgr.write("-"); lgr.write("\n"); }
#define ELAPSED(st, en)			( duration_cast<duration<double>>(en - st).count() )
#define ELAPSED_MS(st, en)		( duration_cast<duration<double, std::milli>>(en - st).count() )
#define ELAPSED_NS(st, en)		( duration_cast<duration<double, std::nano>>(en - st).count() )
#define NOINLINE				__declspec(noinline)
#define KB(x)					(x << 10)
#define MB(x)					(x << 20)
#define GB(x)					(x << 30)
#define ROUND_UP(x,R)			((  ((x) + (R)-1) / (R) ) * (R) )
#define ROUND_DOWN(x, s)		((x) & ~((s)-1))
#define NINE_BIT_MASK			((1<<9) - 1)
#define IS_T_K32				(std::is_same<T, ui>::value)
#define IS_T_K64				(std::is_same<T, ui64>::value)
#define IS_T_K32V32				(std::is_same<T, k32v32>::value)
#define IS_T_K64V64				(std::is_same<T, k64v64>::value)
#define IS_R_SSE				(std::is_same<R, sse>::value)
#define IS_R_AVX2				(std::is_same<R, avx2>::value)
#define IS_R_AVX512				(std::is_same<R, avx512>::value)

inline uint64 RoundUp(ui64 value, ui64 base) {
	return ((value + base - 1) / base) * base;
}

inline uint64 RoundDown(ui64 value, ui64 base) {
	return (value / base) * base;
}