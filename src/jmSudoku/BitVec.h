
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

#define __SSE2__

// For SSE2, SSE3, SSSE3, SSE 4.1, AVX, AVX2
#if defined(_MSC_VER)
#include "msvc_x86intrin.h"
#else
#include <x86intrin.h>
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

    BitVec16x08 & operator = (const BitVec16x08 & right) {
        this->xmm128 = right.xmm128;
        return *this;
    }

    BitVec16x08 & operator = (const __m128i & right) {
        this->xmm128 = right;
        return *this;
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
        ones.setAllOnes;
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

    BitVec16x16 & operator = (const BitVec16x16 & right) {
        this->low = right.low;
        this->high = right.high;
        return *this;
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
};

#else // __AVX2__

struct BitVec16x16 {
    __m256i xmm256;

    BitVec16x16() noexcept {}
    BitVec16x16(__m256i _xmm256) noexcept : xmm256(_xmm256) {}
    BitVec16x16(__m128i low, __m128i high) noexcept
        : xmm256(_mm256_setr_m128i(high, low)) {}

    // non-explicit conversions intended
    BitVec16x16(const BitVec16x16 & src) noexcept = default;
    
    BitVec16x16(const BitVec16x08 & low, const BitVec16x08 & high) noexcept
        : xmm256(_mm256_set_m128i(high.xmm128, low.xmm128)) {}

    BitVec16x16(uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
                uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
                uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
                uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15,
                uint8_t c16, uint8_t c17, uint8_t c18, uint8_t c19,
                uint8_t c20, uint8_t c21, uint8_t c22, uint8_t c23,
                uint8_t c24, uint8_t c25, uint8_t c26, uint8_t c27,
                uint8_t c28, uint8_t c29, uint8_t c30, uint8_t c31) :
            xmm256(_mm256_setr_epi8(
                   c00, c01, c02, c03, c04, c05, c06, c07,
                   c08, c09, c10, c11, c12, c13, c14, c15,
                   c16, c17, c18, c19, c20, c21, c22, c23,
                   c24, c25, c26, c27, c28, c29, c30, c31)) {}

    BitVec16x16(uint16_t w00, uint16_t w01, uint16_t w02, uint16_t w03,
                uint16_t w04, uint16_t w05, uint16_t w06, uint16_t w07,
                uint16_t w08, uint16_t w09, uint16_t w10, uint16_t w11,
                uint16_t w12, uint16_t w13, uint16_t w14, uint16_t w15) :
            xmm256(_mm256_setr_epi16(
                   w00, w01, w02, w03, w04, w05, w06, w07,
                   w08, w09, w10, w11, w12, w13, w14, w15)) {}

    BitVec16x16(uint32_t i00, uint32_t i01, uint32_t i02, uint32_t i03,
                uint32_t i04, uint32_t i05, uint32_t i06, uint32_t i07) :
            xmm256(_mm256_setr_epi32(i00, i01, i02, i03, i04, i05, i06, i07)) {}

    BitVec16x16(uint64_t q00, uint64_t q01, uint64_t q02, uint64_t q03) :
            xmm256(_mm256_setr_epi64x(q00, q01, q02, q03)) {}

    BitVec16x16 & operator = (const BitVec16x16 & right) {
        this->xmm256 = right.xmm256;
        return *this;
    }

    BitVec16x16 & operator = (const __m256i & right) {
        this->xmm256 = right;
        return *this;
    }

    void loadAligned(const void * mem_aligned_32) {
        const __m256i * mem_256i = (const __m256i *)mem_aligned_32;
        this->xmm256 = _mm256_load_si256(mem_256i);
    }

    void loadUnaligned(const void * mem_unaligned) {
        const __m256i * mem_256i = (const __m256i *)mem_unaligned;
        this->xmm256 = _mm256_loadu_si256(mem_256i);
    }

    void saveAligned(void * mem_aligned_32) const {
        __m256i * mem_256i = (__m256i *)mem_aligned_32;
        _mm256_store_si256(mem_256i, this->xmm256);
    }

    void saveUnaligned(void * mem_unaligned) const {
        __m256i * mem_256i = (__m256i *)mem_unaligned;
        _mm256_storeu_si256(mem_256i, this->xmm256);
    }

    bool operator == (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->xmm256);
        tmp._xor(other);
        return (tmp.isAllZeros());
    }

    bool operator != (const BitVec16x16 & other) const {
        BitVec16x16 tmp(this->xmm256);
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
        this->xmm256 = _mm256_and_si256(this->xmm256, vec.xmm256);
    }

    void _and_not(const BitVec16x16 & vec) {
        this->xmm256 = _mm256_andnot_si256(this->xmm256, vec.xmm256);
    }

    void _or(const BitVec16x16 & vec) {
        this->xmm256 = _mm256_or_si256(this->xmm256, vec.xmm256);
    }

    void _xor(const BitVec16x16 & vec) {
        this->xmm256 = _mm256_xor_si256(this->xmm256, vec.xmm256);
    }

    // Logical not: !
    void _not() {
        __m256i zero = _mm256_setzero_si256();
        this->xmm256 = _mm256_andnot_si256(zero, this->xmm256);
    }

    // fill
    void fill_u8(uint8_t value) {
        this->xmm256 = _mm256_set1_epi8(value);
    }

    void fill_u16(uint16_t value) {
        this->xmm256 = _mm256_set1_epi16(value);
    }

    void fill_u32(uint32_t value) {
        this->xmm256 = _mm256_set1_epi32(value);
    }

    void fill_u64(uint64_t value) {
        this->xmm256 = _mm256_set1_epi64x(value);
    }

    // All zeros or all ones
    void setAllZeros() {
        this->xmm256 = _mm256_setzero_si256();
    }

    void setAllOnes() {
        this->xmm256 = _mm256_andnot_si256(this->xmm256, this->xmm256);
    }

    bool isAllZeros() const {
        return (_mm256_test_all_zeros(this->xmm256, this->xmm256) != 0);
    }

    bool isAllOnes() const {
        return (_mm256_test_all_ones(this->xmm256) != 0);
    }

    bool hasAnyZero() const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
        return (_mm256_cmp_epi16_mask(this->xmm256, _mm256_setzero_si256(), _MM_CMPINT_EQ) != 0);
#else
        BitVec16x16 which_is_equal_zero = whichIsZeros();
        return (_mm256_movemask_epi8(which_is_equal_zero.xmm256) != 0);
#endif
    }

    bool hasAnyOne() const {
#if defined(__AVX512VL__) && defined(__AVX512BW__)
        return (_mm256_cmp_epi16_mask(this->xmm256, _mm256_setzero_si256(), _MM_CMPINT_GT) != 0);
#else
        BitVec16x16 which_is_non_zero = whichIsNonZero();
        return (_mm256_movemask_epi8(which_is_non_zero.xmm256) != 0);
#endif
    }

    BitVec16x16 whichIsEqual(const BitVec16x16 & other) const {
        return _mm256_cmpeq_epi16(this->xmm256, other.xmm256);
    }

    BitVec16x16 whichIsZeros() const {
        return _mm256_cmpeq_epi16(this->xmm256, _mm256_setzero_si256());
    }

    BitVec16x16 whichIsNonZero() const {
        return _mm256_cmpgt_epi16(this->xmm256, _mm256_setzero_si256());
    }

    BitVec16x16 whichIsOnes() const {
        __m256i ones;
        return _mm256_cmpeq_epi16(this->xmm256, _mm256_andnot_si256(ones, ones));
    }

    BitVec16x16 popcount16() const {
#if defined(__AVX512BITALG__) && defined(__AVX512VL__)
        return _mm256_popcnt_epi16(this->xmm256);
#else
        __m256i lookup  = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                                           0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
        __m256i mask4   = _mm256_set1_epi16(0x0F);
        __m256i sum_0_3 = _mm256_shuffle_epi8(lookup, _mm256_and_si256(this->xmm256, mask4));
        __m256i sum_4_7 = _mm256_shuffle_epi8(lookup, _mm256_srli_epi16(this->xmm256, 4));
        __m256i sum_0_7 = _mm256_add_epi16(sum_0_3, sum_4_7);
        return _mm256_add_epi16(sum_0_7, _mm256_srli_epi16(this->xmm256, 8));
#endif
    }

    void popcount16(void * output) const {
        BitVec16x16 counts16 = this->popcount16();
        counts16.saveAligned(output);
    }

    void popcount16_unaligned(void * output) const {
        BitVec16x16 counts16 = this->popcount16();
        counts16.saveUnaligned(output);
    }
};

#endif // !__AVX2__

} // namespace jmSudoku

#endif // JSTD_BITVEC_H