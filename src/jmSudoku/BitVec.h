
#ifndef JSTD_BITVEC_H
#define JSTD_BITVEC_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <bitset>
#include <cstring>          // For std::memset()
#include <initializer_list>
#include <type_traits>

//#define __SSE2__
//#define __AVX__
//#define __AVX2__

// For SSE2, SSE3, SSSE3, SSE 4.1, AVX, AVX2
#if defined(_MSC_VER)
#include "msvc_x86intrin.h"
#else
#include <x86intrin.h>
#define _mm_setr_epi64x(high, low) \
            _mm_setr_epi64(_mm_cvtsi64_m64(high), _mm_cvtsi64_m64(low))
#define _mm256_test_all_zeros(mask, val) \
            _mm256_testz_si256((mask), (val))
#define _mm256_test_all_ones(val) \
            _mm256_testc_si256((val), _mm256_cmpeq_epi32((val), (val)))
#endif // _MSC_VER

namespace jmSudoku {

union alignas(16) IntVec128 {
    int8_t   i8[16];
    uint8_t  u8[16];
    int16_t  i16[8];
    uint16_t u16[8];
    int32_t  i32[4];
    uint32_t u32[4];
    int64_t  i64[2];
    uint64_t u64[2];
};

union alignas(32) IntVec256 {
    int8_t   i8[32];
    uint8_t  u8[32];
    int16_t  i16[16];
    uint16_t u16[16];
    int32_t  i32[8];
    uint32_t u32[8];
    int64_t  i64[4];
    uint64_t u64[4];
};

#if defined(__SSE2__) || defined(__SSE3__) || defined(__SSSE3__) || defined(__SSE4A__) || defined(__SSE4a__) \
 || defined(__SSE4_1__) || defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512VL__)

struct BitVec16x08 {
    __m128i xmm128;

    BitVec16x08() noexcept {}
    // non-explicit conversions intended
    BitVec16x08(const __m128i & m128i) noexcept : xmm128(m128i) {}
    BitVec16x08(const BitVec16x08 & src) noexcept : xmm128(src.xmm128) {}

    BitVec16x08(uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
                uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
                uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
                uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15) :
            xmm128(_mm_setr_epi8(c00, c01, c02, c03, c04, c05, c06, c07,
                                 c08, c09, c10, c11, c12, c13, c14, c15)) {}

    BitVec16x08(uint16_t w00, uint16_t w01, uint16_t w02, uint16_t w03,
                uint16_t w04, uint16_t w05, uint16_t w06, uint16_t w07) :
            xmm128(_mm_setr_epi16(w00, w01, w02, w03, w04, w05, w06, w07)) {}

    BitVec16x08(uint32_t i00, uint32_t i01, uint32_t i02, uint32_t i03) :
            xmm128(_mm_setr_epi32(i00, i01, i02, i03)) {}

    BitVec16x08(uint64_t q00, uint64_t q01) :
            xmm128(_mm_setr_epi64x(q00, q01)) {}

    BitVec16x08 & operator = (const BitVec16x08 & right) {
        this->xmm128 = right.xmm128;
        return *this;
    }

    BitVec16x08 & operator = (const __m128i & right) {
        this->xmm128 = right;
        return *this;
    }

    void loadAligned(const void * mem_addr) {
        const __m128i * mem_128i = (const __m128i *)mem_addr;
        this->xmm128 = _mm_load_si128(mem_128i);
    }

    void loadUnaligned(const void * mem_addr) {
        const __m128i * mem_128i = (const __m128i *)mem_addr;
        this->xmm128 = _mm_loadu_si128(mem_128i);
    }

    void saveAligned(void * mem_addr) const {
        __m128i * mem_128i = (__m128i *)mem_addr;
        _mm_store_si128(mem_128i, this->xmm128);
    }

    void saveUnaligned(void * mem_addr) const {
        __m128i * mem_128i = (__m128i *)mem_addr;
        _mm_storeu_si128(mem_128i, this->xmm128);
    }

    bool operator == (const BitVec16x08 & other) const {
        BitVec16x08 tmp(this->xmm128);
        tmp._xor(other);
        return (tmp.isAllZeros());
    }

    bool operator != (const BitVec16x08 & other) const {
        BitVec16x08 tmp(this->xmm128);
        tmp._xor(other);
        return !(tmp.isAllZeros());
    }

    // Logical operation
    BitVec16x08 & operator & (const BitVec16x08 & vec) {
        this->_and(vec.xmm128);
        return *this;
    }

    BitVec16x08 & operator | (const BitVec16x08 & vec) {
        this->_or(vec.xmm128);
        return *this;
    }

    BitVec16x08 & operator ^ (const BitVec16x08 & vec) {
        this->_xor(vec.xmm128);
        return *this;
    }

    BitVec16x08 & operator ~ () {
        this->_not();
        return *this;
    }

    BitVec16x08 & operator ! () {
        this->_not();
        return *this;
    }

    // Logical operation
    BitVec16x08 & operator &= (const BitVec16x08 & vec) {
        this->_and(vec.xmm128);
        return *this;
    }

    BitVec16x08 & operator |= (const BitVec16x08 & vec) {
        this->_or(vec.xmm128);
        return *this;
    }

    BitVec16x08 & operator ^= (const BitVec16x08 & vec) {
        this->_xor(vec.xmm128);
        return *this;
    }

    // Logical operation
    void _and(const BitVec16x08 & vec) {
        this->xmm128 = _mm_and_si128(this->xmm128, vec.xmm128);
    }

    void _and_not(const BitVec16x08 & vec) {
        this->xmm128 = _mm_andnot_si128(this->xmm128, vec.xmm128);
    }

    void _or(const BitVec16x08 & vec) {
        this->xmm128 = _mm_or_si128(this->xmm128, vec.xmm128);
    }

    void _xor(const BitVec16x08 & vec) {
        this->xmm128 = _mm_xor_si128(this->xmm128, vec.xmm128);
    }

    // Logical operation
    void _and(__m128i value) {
        this->xmm128 = _mm_and_si128(this->xmm128, value);
    }

    void _and_not(__m128i value) {
        this->xmm128 = _mm_andnot_si128(this->xmm128, value);
    }

    void _or(__m128i value) {
        this->xmm128 = _mm_or_si128(this->xmm128, value);
    }

    void _xor(__m128i value) {
        this->xmm128 = _mm_xor_si128(this->xmm128, value);
    }

    // Logical not: !
    void _not() {
        __m128i zero = _mm_setzero_si128();
        this->xmm128 = _mm_andnot_si128(zero, this->xmm128);
    }

    // fill
    void fill_u8(uint8_t value) {
        this->xmm128 = _mm_set1_epi8(value);       // SSE2
    }

    void fill_u16(uint16_t value) {
        this->xmm128 = _mm_set1_epi16(value);      // SSE2
    }

    void fill_u32(uint32_t value) {
        this->xmm128 = _mm_set1_epi32(value);      // SSE2
    }

    void fill_u64(uint64_t value) {
        this->xmm128 = _mm_set1_epi64x(value);     // SSE2
    }

    // All zeros or all ones
    void setAllZeros() {
        this->xmm128 = _mm_setzero_si128();        // SSE2
    }

    void setAllOnes() {
        this->xmm128 = _mm_andnot_si128(this->xmm128, this->xmm128);
    }

    bool isAllZeros() const {
#ifdef __SSE4_1__
        return (_mm_test_all_zeros(this->xmm128, this->xmm128) != 0);
#else
        return (_mm_movemask_epi8(whichIsEqual(_mm_setzero_si128()).xmm128) == 0xffff);
#endif
    }

    bool isAllOnes() const {
#ifdef __SSE4_1__
        return (_mm_test_all_ones(this->xmm128) != 0);
#else
        BitVec16x08 ones;
        ones.setAllOnes();
        BitVec16x08 compare_mask = whichIsEqual(ones);
        return (_mm_movemask_epi8(compare_mask.xmm128) == 0xffff);
#endif
    }

    BitVec16x08 whichIsEqual(const BitVec16x08 & other) const {
        return _mm_cmpeq_epi16(this->xmm128, other.xmm128);
    }

    BitVec16x08 whichIsZeros() const {
        return _mm_cmpeq_epi16(this->xmm128, _mm_setzero_si128());
    }

    BitVec16x08 whichIsNonZero() const {
        return _mm_cmpgt_epi16(this->xmm128, _mm_setzero_si128());
    }

    BitVec16x08 whichIsOnes() const {
        __m128i ones;
        return _mm_cmpeq_epi16(this->xmm128, _mm_andnot_si128(ones, ones));
    }

    BitVec16x08 popcount16() const {
#if defined(__SSSE3__)
        __m128i lookup  = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
        __m128i mask4   = _mm_set1_epi16(0x0F);
        __m128i sum_0_3 = _mm_shuffle_epi8(lookup, _mm_and_si128(this->xmm128, mask4));
        __m128i sum_4_7 = _mm_shuffle_epi8(lookup, _mm_srli_epi16(this->xmm128, 4));
        __m128i sum_0_7 = _mm_add_epi16(sum_0_3, sum_4_7);
        __m128i result  = _mm_add_epi16(sum_0_7, _mm_srli_epi16(this->xmm128, 8));
        return result;
#else
        // SSE2 version from https://www.hackersdelight.org/hdcodetxt/pop.c.txt
        __m128i mask1 = _mm_set1_epi8(0x77);
        __m128i mask2 = _mm_set1_epi8(0x0f);
        __m128i mask3 = _mm_set1_epi16(0xff);
        __m128i x = this->xmm128;
        __m128i n = _mm_and_si128(mask1, _mm_srli_epi64(x, 1));
        x = _mm_sub_epi8(x, n);
        n = _mm_and_si128(mask1, _mm_srli_epi64(n, 1));
        x = _mm_sub_epi8(x, n);
        n = _mm_and_si128(mask1, _mm_srli_epi64(n, 1));
        x = _mm_sub_epi8(x, n);
        x = _mm_add_epi8(x, _mm_srli_epi16(x, 4));
        x = _mm_and_si128(mask2, x);
        // Tip: _mm_bsrli_si128() is missing on MSVC (Windows)
        x = _mm_add_epi16(_mm_and_si128(x, mask3),
                          _mm_and_si128(_mm_srli_si128(x, 1), mask3));
        return x;
#endif
    }

    void popcount16(void * mem_addr) const {
        BitVec16x08 popcnt16 = this->popcount16();
        popcnt16.saveAligned(mem_addr);
    }

    void popcount16_unaligned(void * mem_addr) const {
        BitVec16x08 popcnt16 = this->popcount16();
        popcnt16.saveUnaligned(mem_addr);
    }

    template <size_t MaxBits>
    BitVec16x08 _minpos8() const {
#if defined(__SSE4_1__)
        //
        // See: https://blog.csdn.net/weixin_34378767/article/details/86257834
        //
        if (MaxBits <= 8) {
            __m128i zero = _mm_setzero_si128();
            __m128i low64 = _mm_unpacklo_epi8(this->xmm128, zero);
            __m128i minpos128 = _mm_minpos_epu16(low64);
            BitVec16x08 minpos = _mm_packus_epi16(minpos128, _mm_setzero_si128());
            return minpos;
        }
        else if (MaxBits == 9) {
            __m128i zero = _mm_setzero_si128();
            __m128i low64  = _mm_unpacklo_epi8(this->xmm128, zero);
            __m128i high64 = _mm_unpackhi_epi8(this->xmm128, zero);
            BitVec16x08 minpos = _mm_min_epu16(_mm_minpos_epu16(low64), high64);
            return minpos;
        }
        else {
            __m128i zero = _mm_setzero_si128();
            __m128i low64  = _mm_unpacklo_epi8(this->xmm128, zero);
            __m128i high64 = _mm_unpackhi_epi8(this->xmm128, zero);
            BitVec16x08 minpos = _mm_min_epu16(_mm_minpos_epu16(low64), _mm_minpos_epu16(high64));
            return minpos;
        }
#else
        //
        // Horizontal minimum and maximum using SSE
        // See: https://stackoverflow.com/questions/22256525/horizontal-minimum-and-maximum-using-sse
        //
        if (MaxBits <= 8) {
            BitVec16x08 numbers = this->xmm128;
            numbers = _mm_min_epu8(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(1, 0, 3, 2)));
            numbers = _mm_min_epu8(numbers, _mm_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

            BitVec16x08 minpos = _mm_min_epu8(numbers, _mm_srli_epi16(numbers, 8));
            return minpos;
        }
        else {
            BitVec16x08 numbers = this->xmm128;
            numbers = _mm_min_epu8(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
            numbers = _mm_min_epu8(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
            numbers = _mm_min_epu8(numbers, _mm_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

            BitVec16x08 minpos = _mm_min_epu8(numbers, _mm_srli_epi16(numbers, 8));
            return minpos;
        }
#endif
    }

    template <size_t MaxBits>
    int minpos8(BitVec16x08 & minpos) const {
        minpos = this->_minpos8<MaxBits>();
#if defined(__SSE4_1__)
        return _mm_extract_epi16(minpos.xmm128, 0);
#else
        return _mm_extract_epi8(minpos.xmm128, 0);
#endif
    }

    template <size_t MaxBits>
    void _minpos16(BitVec16x08 & minpos) const {
#if defined(__SSE4_1__)
        minpos = _mm_minpos_epu16(this->xmm128);
#else
        if (MaxBits <= 8) {
            __m128i numbers = this->xmm128;
            numbers = _mm_min_epu16(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(1, 0, 3, 2)));

            // The minimum number of total 16
            minpos = _mm_min_epu16(numbers, _mm_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
        }
        else {
            __m128i numbers = this->xmm128;
            numbers = _mm_min_epu16(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
            numbers = _mm_min_epu16(numbers, _mm_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

            // The minimum number of total 16
            minpos = _mm_min_epu16(numbers, _mm_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
        }
#endif
    }

    template <size_t MaxBits>
    int minpos16(BitVec16x08 & minpos) const {
        this->_minpos16<MaxBits>(minpos);
        return _mm_extract_epi16(minpos.xmm128, 0);
    }
};

#endif // >= SSE2

#if !defined(__AVX2__)

struct BitVec16x16 {
    BitVec16x08 low;
    BitVec16x08 high;

    BitVec16x16() noexcept : low(), high() {}

    // non-explicit conversions intended
    BitVec16x16(const BitVec16x16 & src) noexcept = default;

    BitVec16x16(const BitVec16x08 & _low, const BitVec16x08 & _high) noexcept
        : low(_low), high(_high) {}

    BitVec16x16(uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
                uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
                uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
                uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15,
                uint8_t c16, uint8_t c17, uint8_t c18, uint8_t c19,
                uint8_t c20, uint8_t c21, uint8_t c22, uint8_t c23,
                uint8_t c24, uint8_t c25, uint8_t c26, uint8_t c27,
                uint8_t c28, uint8_t c29, uint8_t c30, uint8_t c31) :
            low(c00, c01, c02, c03, c04, c05, c06, c07,
                c08, c09, c10, c11, c12, c13, c14, c15),
            high(c16, c17, c18, c19, c20, c21, c22, c23,
                 c24, c25, c26, c27, c28, c29, c30, c31) {}

    BitVec16x16(uint16_t w00, uint16_t w01, uint16_t w02, uint16_t w03,
                uint16_t w04, uint16_t w05, uint16_t w06, uint16_t w07,
                uint16_t w08, uint16_t w09, uint16_t w10, uint16_t w11,
                uint16_t w12, uint16_t w13, uint16_t w14, uint16_t w15) :
            low(w00, w01, w02, w03, w04, w05, w06, w07),
            high(w08, w09, w10, w11, w12, w13, w14, w15) {}

    BitVec16x16(uint32_t i00, uint32_t i01, uint32_t i02, uint32_t i03,
                uint32_t i04, uint32_t i05, uint32_t i06, uint32_t i07) :
            low(i00, i01, i02, i03), high(i04, i05, i06, i07)  {}

    BitVec16x16(uint64_t q00, uint64_t q01, uint64_t q02, uint64_t q03) :
            low(q00, q01), high(q02, q03) {}

    BitVec16x16 & mergeFrom(const BitVec16x08 & low, const BitVec16x08 & high) {
        this->low = _low;
        this->high = _high;
        return *this;
    }

    void splitTo(BitVec16x08 & _low, BitVec16x08 & _high) const {
        _low = this->low;
        _high = this->high;
    }

    BitVec16x16 & operator = (const BitVec16x16 & right) {
        this->low = right.low;
        this->high = right.high;
        return *this;
    }

    void loadAligned(const void * mem_addr) {
        const __m128i * mem_128i_low = (const __m128i *)mem_addr;
        const __m128i * mem_128i_high = ((const __m128i *)mem_addr) + 1;
        this->low.loadAligned(mem_128i_low);
        this->low.loadAligned(mem_128i_high);
    }

    void loadUnaligned(const void * mem_addr) {
        const __m128i * mem_128i_low = (const __m128i *)mem_addr;
        const __m128i * mem_128i_high = ((const __m128i *)mem_addr) + 1;
        this->low.loadUnaligned(mem_128i_low);
        this->low.loadUnaligned(mem_128i_high);
    }

    void saveAligned(void * mem_addr) const {
        __m128i * mem_128i_low = (__m128i *)mem_addr;
        __m128i * mem_128i_high = ((__m128i *)mem_addr) + 1;
        this->low.saveAligned(mem_128i_low);
        this->low.saveAligned(mem_128i_high);
    }

    void saveUnaligned(void * mem_addr) const {
        __m128i * mem_128i_low = (__m128i *)mem_addr;
        __m128i * mem_128i_high = ((__m128i *)mem_addr) + 1;
        this->low.saveUnaligned(mem_128i_low);
        this->low.saveUnaligned(mem_128i_high);
    }

    bool operator == (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->low, this->high);
        tmp._xor(other);
        return (tmp.isAllZeros());
    }

    bool operator != (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->low, this->high);
        tmp._xor(other);
        return !(tmp.isAllZeros());
    }

    // Logical operation
    BitVec16x16 & operator & (const BitVec16x16 & vec) {
        this->_and(vec);
        return *this;
    }

    BitVec16x16 & operator | (const BitVec16x16 & vec) {
        this->_or(vec);
        return *this;
    }

    BitVec16x16 & operator ^ (const BitVec16x16 & vec) {
        this->_xor(vec);
        return *this;
    }

    BitVec16x16 & operator ~ () {
        this->_not();
        return *this;
    }

    BitVec16x16 & operator ! () {
        this->_not();
        return *this;
    }

    // Logical operation
    BitVec16x16 & operator &= (const BitVec16x16 & vec) {
        this->_and(vec);
        return *this;
    }

    BitVec16x16 & operator |= (const BitVec16x16 & vec) {
        this->_or(vec);
        return *this;
    }

    BitVec16x16 & operator ^= (const BitVec16x16 & vec) {
        this->_xor(vec);
        return *this;
    }

    // Logical operation
    void _and(const BitVec16x16 & vec) {
        this->low._and(vec.low);
        this->high._and(vec.low);
    }

    void _and_not(const BitVec16x16 & vec) {
        this->low._and_not(vec.low);
        this->high._and_not(vec.low);
    }

    void _or(const BitVec16x16 & vec) {
        this->low._or(vec.low);
        this->high._or(vec.low);
    }

    void _xor(const BitVec16x16 & vec) {
        this->low._xor(vec.low);
        this->high._xor(vec.low);
    }

    // Logical not: !
    void _not() {
        this->low._not();
        this->high._not();
    }

    // fill
    void fill_u8(uint8_t value) {
        this->low.fill_u8(value);
        this->high.fill_u8(value);
    }

    void fill_u16(uint16_t value) {
        this->low.fill_u16(value);
        this->high.fill_u16(value);
    }

    void fill_u32(uint32_t value) {
        this->low.fill_u32(value);
        this->high.fill_u32(value);
    }

    void fill_u64(uint64_t value) {
        this->low.fill_u64(value);
        this->high.fill_u64(value);
    }

    // All zeros or all ones
    void setAllZeros() {
        this->low.setAllZeros();
        this->high.setAllZeros();
    }

    void setAllOnes() {
        this->low.setAllOnes();
        this->high.setAllOnes();
    }

    bool isAllZeros() const {
        return (this->low.isAllZeros() && this->high.isAllZeros());
    }

    bool isAllOnes() const {
        return (this->low.isAllOnes() && this->high.isAllOnes());
    }

    bool hasAnyZero() const {
        return (this->low.hasAnyZero() && this->high.hasAnyZero());
    }

    bool hasAnyOne() const {
        return (this->low.hasAnyOne() && this->high.hasAnyOne());
    }

    BitVec16x16 whichIsEqual(const BitVec16x16 & other) const {
        return BitVec16x16(this->low.whichIsEqual(other.low), this->high.whichIsEqual(other.high));
    }

    BitVec16x16 whichIsZeros() const {
        return BitVec16x16(this->low.whichIsZeros(), this->high.whichIsZeros());
    }

    BitVec16x16 whichIsNonZero() const {
        return BitVec16x16(this->low.whichIsNonZero(), this->high.whichIsNonZero());
    }

    BitVec16x16 whichIsOnes() const {
        return BitVec16x16(this->low.whichIsOnes(), this->high.whichIsOnes());
    }

    template <size_t MaxBits>
    BitVec16x16 popcount16() const {
        return BitVec16x16(this->low.popcount16<MaxBits>(), this->high.popcount16<MaxBits>());
    }

    template <size_t MaxBits>
    void popcount16(void * mem_addr) const {
        BitVec16x16 popcnt16 = this->popcount16<MaxBits>();
        popcnt16.saveAligned(mem_addr);
    }

    template <size_t MaxBits>
    void popcount16_unaligned(void * mem_addr) const {
        BitVec16x16 popcnt16 = this->popcount16<MaxBits>();
        popcnt16.saveUnaligned(mem_addr);
    }

    template <size_t MaxBits>
    void _minpos8(BitVec16x16 & minpos) const {
        if (MaxBits <= 8) {
            BitVec16x08 low_minpos;
            this->low._minpos8(low_minpos);
            minpos.mergeFrom(low_minpos, low_minpos);
        }
        else if (MaxBits == 9) {
            BitVec16x08 low_minpos;
            this->low._minpos8(low_minpos);
            BitVec16x08 minpos128 = _mm_min_epu8(this->low, this->high);
            minpos.mergeFrom(minpos128, minpos128);
        }
        else {
            BitVec16x08 low_minpos, high_minpos;
            this->low._minpos8(low_minpos);
            this->high._minpos8(high_minpos);
            BitVec16x08 minpos128 = _mm_min_epu8(this->low, this->high);
            minpos.mergeFrom(minpos128, minpos128);
        }
    }
};

#else // __AVX2__

struct BitVec16x16 {
    __m256i ymm256;

    BitVec16x16() noexcept {}
    BitVec16x16(__m256i _ymm256) noexcept : ymm256(_ymm256) {}
    BitVec16x16(__m128i low, __m128i high) noexcept
        : ymm256(_mm256_setr_m128i(high, low)) {}

    // non-explicit conversions intended
    BitVec16x16(const BitVec16x16 & src) noexcept = default;
    
    BitVec16x16(const BitVec16x08 & low, const BitVec16x08 & high) noexcept
        : ymm256(_mm256_set_m128i(high.xmm128, low.xmm128)) {}

    BitVec16x16(uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
                uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
                uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
                uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15,
                uint8_t c16, uint8_t c17, uint8_t c18, uint8_t c19,
                uint8_t c20, uint8_t c21, uint8_t c22, uint8_t c23,
                uint8_t c24, uint8_t c25, uint8_t c26, uint8_t c27,
                uint8_t c28, uint8_t c29, uint8_t c30, uint8_t c31) :
            ymm256(_mm256_setr_epi8(
                   c00, c01, c02, c03, c04, c05, c06, c07,
                   c08, c09, c10, c11, c12, c13, c14, c15,
                   c16, c17, c18, c19, c20, c21, c22, c23,
                   c24, c25, c26, c27, c28, c29, c30, c31)) {}

    BitVec16x16(uint16_t w00, uint16_t w01, uint16_t w02, uint16_t w03,
                uint16_t w04, uint16_t w05, uint16_t w06, uint16_t w07,
                uint16_t w08, uint16_t w09, uint16_t w10, uint16_t w11,
                uint16_t w12, uint16_t w13, uint16_t w14, uint16_t w15) :
            ymm256(_mm256_setr_epi16(
                   w00, w01, w02, w03, w04, w05, w06, w07,
                   w08, w09, w10, w11, w12, w13, w14, w15)) {}

    BitVec16x16(uint32_t i00, uint32_t i01, uint32_t i02, uint32_t i03,
                uint32_t i04, uint32_t i05, uint32_t i06, uint32_t i07) :
            ymm256(_mm256_setr_epi32(i00, i01, i02, i03, i04, i05, i06, i07)) {}

    BitVec16x16(uint64_t q00, uint64_t q01, uint64_t q02, uint64_t q03) :
            ymm256(_mm256_setr_epi64x(q00, q01, q02, q03)) {}

    BitVec16x16 & mergeFrom(const BitVec16x08 & low, const BitVec16x08 & high) {
        this->ymm256 = _mm256_set_m128i(high.xmm128, low.xmm128);
        return *this;
    }

    void castTo(BitVec16x08 & low) const {
        low = _mm256_castsi256_si128(this->ymm256);
    }

    void splitTo(BitVec16x08 & low, BitVec16x08 & high) const {
        low = _mm256_castsi256_si128(this->ymm256);
        high = _mm256_castsi256_si128(_mm256_bsrli_epi128(this->ymm256, 8));
    }

    BitVec16x08 getLow() const {
#if 1
        return _mm256_castsi256_si128(this->ymm256);
#else
        return _mm256_extracti128_si256(vec, 0);
#endif
    }

    BitVec16x08 getHigh() const {
#if 1
        return _mm256_castsi256_si128(_mm256_bsrli_epi128(this->ymm256, 8));
#else
        return _mm256_extracti128_si256(vec, 1);
#endif
    }

    BitVec16x16 & operator = (const BitVec16x16 & right) {
        this->ymm256 = right.ymm256;
        return *this;
    }

    BitVec16x16 & operator = (const __m256i & right) {
        this->ymm256 = right;
        return *this;
    }

    void loadAligned(const void * mem_addr) {
        const __m256i * mem_256i = (const __m256i *)mem_addr;
        this->ymm256 = _mm256_load_si256(mem_256i);
    }

    void loadUnaligned(const void * mem_addr) {
        const __m256i * mem_256i = (const __m256i *)mem_addr;
        this->ymm256 = _mm256_loadu_si256(mem_256i);
    }

    void saveAligned(void * mem_addr) const {
        __m256i * mem_256i = (__m256i *)mem_addr;
        _mm256_store_si256(mem_256i, this->ymm256);
    }

    void saveUnaligned(void * mem_addr) const {
        __m256i * mem_256i = (__m256i *)mem_addr;
        _mm256_storeu_si256(mem_256i, this->ymm256);
    }

    bool operator == (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->ymm256);
        tmp._xor(other);
        return (tmp.isAllZeros());
    }

    bool operator != (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->ymm256);
        tmp._xor(other);
        return (tmp.isAllZeros());
    }

    // Logical operation
    BitVec16x16 & operator & (const BitVec16x16 & vec) {
        this->_and(vec);
        return *this;
    }

    BitVec16x16 & operator | (const BitVec16x16 & vec) {
        this->_or(vec);
        return *this;
    }

    BitVec16x16 & operator ^ (const BitVec16x16 & vec) {
        this->_xor(vec);
        return *this;
    }

    BitVec16x16 & operator ~ () {
        this->_not();
        return *this;
    }

    BitVec16x16 & operator ! () {
        this->_not();
        return *this;
    }

    // Logical operation
    BitVec16x16 & operator &= (const BitVec16x16 & vec) {
        this->_and(vec);
        return *this;
    }

    BitVec16x16 & operator |= (const BitVec16x16 & vec) {
        this->_or(vec);
        return *this;
    }

    BitVec16x16 & operator ^= (const BitVec16x16 & vec) {
        this->_xor(vec);
        return *this;
    }

    // Logical operation
    void _and(const BitVec16x16 & vec) {
        this->ymm256 = _mm256_and_si256(this->ymm256, vec.ymm256);
    }

    void _and_not(const BitVec16x16 & vec) {
        this->ymm256 = _mm256_andnot_si256(this->ymm256, vec.ymm256);
    }

    void _or(const BitVec16x16 & vec) {
        this->ymm256 = _mm256_or_si256(this->ymm256, vec.ymm256);
    }

    void _xor(const BitVec16x16 & vec) {
        this->ymm256 = _mm256_xor_si256(this->ymm256, vec.ymm256);
    }

    // Logical not: !
    void _not() {
        __m256i zero = _mm256_setzero_si256();
        this->ymm256 = _mm256_andnot_si256(zero, this->ymm256);
    }

    // fill
    void fill_u8(uint8_t value) {
        this->ymm256 = _mm256_set1_epi8(value);
    }

    void fill_u16(uint16_t value) {
        this->ymm256 = _mm256_set1_epi16(value);
    }

    void fill_u32(uint32_t value) {
        this->ymm256 = _mm256_set1_epi32(value);
    }

    void fill_u64(uint64_t value) {
        this->ymm256 = _mm256_set1_epi64x(value);
    }

    // All zeros or all ones
    void setAllZeros() {
        this->ymm256 = _mm256_setzero_si256();
    }

    void setAllOnes() {
        this->ymm256 = _mm256_andnot_si256(this->ymm256, this->ymm256);
    }

    bool isAllZeros() const {
        return (_mm256_test_all_zeros(this->ymm256, this->ymm256) != 0);
    }

    bool isAllOnes() const {
        return (_mm256_test_all_ones(this->ymm256) != 0);
    }

    bool hasAnyZero() const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
        return (_mm256_cmp_epi16_mask(this->ymm256, _mm256_setzero_si256(), _MM_CMPINT_EQ) != 0);
#else
        BitVec16x16 which_is_equal_zero = whichIsZeros();
        return (_mm256_movemask_epi8(which_is_equal_zero.ymm256) != 0);
#endif
    }

    bool hasAnyOne() const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
        return (_mm256_cmp_epi16_mask(this->ymm256, _mm256_setzero_si256(), _MM_CMPINT_GT) != 0);
#else
        BitVec16x16 which_is_non_zero = whichIsNonZero();
        return (_mm256_movemask_epi8(which_is_non_zero.ymm256) != 0);
#endif
    }

    BitVec16x16 whichIsEqual(const BitVec16x16 & other) const {
        return _mm256_cmpeq_epi16(this->ymm256, other.ymm256);
    }

    BitVec16x16 whichIsZeros() const {
        return _mm256_cmpeq_epi16(this->ymm256, _mm256_setzero_si256());
    }

    BitVec16x16 whichIsNonZero() const {
        return _mm256_cmpgt_epi16(this->ymm256, _mm256_setzero_si256());
    }

    BitVec16x16 whichIsOnes() const {
        __m256i ones;
        return _mm256_cmpeq_epi16(this->ymm256, _mm256_andnot_si256(ones, ones));
    }

    template <size_t MaxBits>
    BitVec16x16 popcount16() const {
#if defined(__AVX512BITALG__) && defined(__AVX512VL__)
        return _mm256_popcnt_epi16(this->ymm256);
#else
        if (MaxBits <= 8) {
            __m256i lookup  = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                                               0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
            __m256i mask4   = _mm256_set1_epi16(0x0F);
            __m256i sum_0_3 = _mm256_shuffle_epi8(lookup, _mm256_and_si256(this->ymm256, mask4));
            __m256i sum_4_7 = _mm256_shuffle_epi8(lookup, _mm256_srli_epi16(this->ymm256, 4));
            __m256i sum_0_7 = _mm256_add_epi16(sum_0_3, sum_4_7);
            __m256i result  = sum_0_7;
            return result;
        }
        else if (MaxBits == 9) {
            __m256i lookup  = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                                               0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
            __m256i mask4   = _mm256_set1_epi16(0x0F);
            __m256i sum_0_3 = _mm256_shuffle_epi8(lookup, _mm256_and_si256(this->ymm256, mask4));
            __m256i sum_4_7 = _mm256_shuffle_epi8(lookup, _mm256_srli_epi16(this->ymm256, 4));
            __m256i sum_0_7 = _mm256_add_epi16(sum_0_3, sum_4_7);
            __m256i result  = _mm256_add_epi16(sum_0_7, _mm256_srli_epi16(this->ymm256, 8));
            return result;
        }
        else {
            __m256i lookup    = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                                                 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
            __m256i mask4     = _mm256_set1_epi16(0x0F);
            __m256i sum_00_03 = _mm256_shuffle_epi8(lookup, _mm256_and_si256(this->ymm256, mask4));
            __m256i sum_04_07 = _mm256_shuffle_epi8(lookup, _mm256_srli_epi16(this->ymm256, 4));
            __m256i sum_00_07 = _mm256_add_epi16(sum_00_03, sum_04_07);
            __m256i high8     = _mm256_srli_epi16(this->ymm256, 8);
            __m256i sum_08_11 = _mm256_shuffle_epi8(lookup, _mm256_and_si256(high8, mask4));
            __m256i sum_12_15 = _mm256_shuffle_epi8(lookup, _mm256_srli_epi16(high8, 4));
            __m256i sum_08_15 = _mm256_add_epi16(sum_08_11, sum_12_15);
            __m256i result    = _mm256_add_epi16(sum_00_07, sum_08_15);
            return result;
        }
#endif // __AVX512__
    }

    template <size_t MaxBits>
    void popcount16(void * mem_addr) const {
        BitVec16x16 popcnt16 = this->popcount16<MaxBits>();
        popcnt16.saveAligned(mem_addr);
    }

    template <size_t MaxBits>
    void popcount16_unaligned(void * mem_addr) const {
        BitVec16x16 popcnt16 = this->popcount16<MaxBits>();
        popcnt16.saveUnaligned(mem_addr);
    }

    template <size_t MaxBits>
    void _minpos8(BitVec16x16 & minpos) const {
        if (MaxBits <= 8) {
            __m256i numbers = this->ymm256;
            numbers = _mm256_min_epu8(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(2, 3, 0, 1)));
            numbers = _mm256_min_epu8(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

            // The minimum number of total 16 x int8_t
            minpos = _mm256_min_epu8(numbers, _mm256_srli_epi16(numbers, 8));
        }
        else if (MaxBits <= 16) {
            __m256i numbers = this->ymm256;
            numbers = _mm256_min_epu8(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
            numbers = _mm256_min_epu8(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
            numbers = _mm256_min_epu8(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

            // The minimum number of total 16 x int8_t
            minpos = _mm256_min_epu8(numbers, _mm256_srli_epi16(numbers, 8));
        }
        else {
            __m256i numbers = this->ymm256;
            numbers = _mm256_min_epu8(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
            numbers = _mm256_min_epu8(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
            numbers = _mm256_min_epu8(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
            numbers = _mm256_min_epu8(numbers, _mm256_srli_epi16(numbers, 8));

            // The minimum number of total 32 x int8_t
            minpos = _mm256_min_epu8(numbers, _mm256_srli_si256(numbers, 16));
        }
    }

    template <size_t MaxBits>
    int minpos8(BitVec16x16 & minpos) const {
        this->_minpos8<MaxBits>(minpos);
#if defined(__AVX2__) && (!defined(_MSC_VER) || (_MSC_VER >= 2000))
        return _mm256_extract_epi16(minpos.ymm256, 0);
#else
        return _mm256_cvtsi256_si32(minpos.ymm256) & 0x000000FFUL;
#endif
    }

    template <size_t MaxBits>
    void _minpos16(BitVec16x16 & minpos) const {
#if defined(__AVX2__)
        __m256i numbers = this->ymm256;
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
        numbers = _mm256_min_epu16(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

        // The minimum number of total 16 x int16_t
        __m256i numbers_high = _mm256_permute4x64_epi64(numbers, _MM_SHUFFLE(1, 0, 3, 2));
        minpos = _mm256_min_epu16(numbers, numbers_high);
#else // !__AVX2__
        BitVec16x08 low, high;
        this->splitTo(low, high);

        BitVec16x16 min_low, min_high;
        low._minpos16<MaxBits>(min_low);
        high._minpos16<MaxBits>(min_high);

        BitVec16x08 minpos128 = _mm_min_epu16(min_low, min_high);
        minpos->mergeFrom(minpos128, minpos128);
#endif // __AVX2__
    }

    template <size_t MaxBits>
    int minpos16(BitVec16x16 & minpos) const {
        this->_minpos16<MaxBits>(minpos);
#if defined(__AVX2__) && (!defined(_MSC_VER) || (_MSC_VER >= 2000))
        return _mm256_extract_epi16(minpos.ymm256, 0);
#else
        return _mm256_cvtsi256_si32(minpos.ymm256) & 0x000000FFUL;
#endif
    }

    template <size_t MaxBits>
    uint32_t minpos16(uint32_t & old_min_num, uint32_t & min_index) const {
        __m256i numbers = this->ymm256;
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
        numbers = _mm256_min_epu16(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

        // The minimum number of total 16 x int16_t
        __m256i numbers_high = _mm256_permute4x64_epi64(numbers, _MM_SHUFFLE(1, 0, 3, 2));
        BitVec16x16 minpos = _mm256_min_epu16(numbers, numbers_high);
#if (!defined(_MSC_VER) || (_MSC_VER >= 2000))
        uint32_t min_num = _mm256_extract_epi16(minpos.ymm256, 0);
#else
        BitVec16x08 minpos128;
        minpos.castTo(minpos128);
        uint32_t min_num = _mm_extract_epi16(minpos128.xmm128, 0);
#endif // _MSC_VER
        if (min_num < old_min_num) {
            old_min_num = min_num;

            // Get the index of minimum number
            __m256i min_num_repeat = _mm256_broadcastw_epi16(_mm256_castsi256_si128(minpos.ymm256));
            __m256i equal_result = _mm256_cmpeq_epi16(this->ymm256, min_num_repeat);

            int equal_mask = _mm256_movemask_epi8(equal_result);
            assert(equal_mask != 0);
            int equal_offset = BitUtils::bsf(equal_mask);
            min_index = equal_offset >> 1;
        }
        return min_num;
    }

    template <size_t MaxBits>
    uint32_t minpos16_and_index(uint32_t & min_index) const {
        __m256i numbers = this->ymm256;
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(3, 2, 3, 2)));
        numbers = _mm256_min_epu16(numbers, _mm256_shuffle_epi32(numbers, _MM_SHUFFLE(1, 1, 1, 1)));
        numbers = _mm256_min_epu16(numbers, _mm256_shufflelo_epi16(numbers, _MM_SHUFFLE(1, 1, 1, 1)));

        // The minimum number of total 16 x int16_t
        __m256i numbers_high = _mm256_permute4x64_epi64(numbers, _MM_SHUFFLE(1, 0, 3, 2));
        BitVec16x16 minpos = _mm256_min_epu16(numbers, numbers_high);

        // Get the index of minimum number
        __m256i min_num_repeat = _mm256_broadcastw_epi16(_mm256_castsi256_si128(minpos.ymm256));
        __m256i equal_result = _mm256_cmpeq_epi16(this->ymm256, min_num_repeat);

        int equal_mask = _mm256_movemask_epi8(equal_result);
        assert(equal_mask != 0);
        int equal_offset = BitUtils::bsf(equal_mask);
        min_index = equal_offset >> 1;

#if (!defined(_MSC_VER) || (_MSC_VER >= 2000))
        uint32_t min_num = _mm256_extract_epi16(minpos.ymm256, 0);
#else
        BitVec16x08 minpos128;
        minpos.castTo(minpos128);
        uint32_t min_num = _mm_extract_epi16(minpos128.xmm128, 0);
#endif // _MSC_VER
        return min_num;
    }
};

#endif // !__AVX2__

} // namespace jmSudoku

#endif // JSTD_BITVEC_H
