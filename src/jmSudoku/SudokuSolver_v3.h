
#ifndef JM_SUDOKU_SOLVER_V3_H
#define JM_SUDOKU_SOLVER_V3_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER)
#define __SSE_4_1__     1
#define __SSE_4_2__     1
#endif

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <cstring>      // For std::memset()
#include <vector>
#include <bitset>

#include <emmintrin.h>      // For SSE 2
#include <tmmintrin.h>      // For SSE 3
#include <smmintrin.h>      // For SSE 4.1

#include "Sudoku.h"
#include "StopWatch.h"

/************************************************

#define SEARCH_MODE_ONE_ANSWER              0
#define SEARCH_MODE_MORE_THAN_ONE_ANSWER    1
#define SEARCH_MODE_ALL_ANSWERS             2

************************************************/

#define V3_SEARCH_MODE      SEARCH_MODE_ONE_ANSWER

namespace jmSudoku {
namespace v3 {

static const size_t kSearchMode = V3_SEARCH_MODE;

template <size_t Capacity>
struct FixedDlxNodeList {
public:
    typedef FixedDlxNodeList<Capacity> this_type;

    static const size_t kCapacity = (Capacity + 1) / 2 * 2;

    uint16_t prev[kCapacity];
    uint16_t next[kCapacity];
    uint16_t up[kCapacity];
    uint16_t down[kCapacity];
    uint16_t row[kCapacity];
    uint16_t col[kCapacity];

    FixedDlxNodeList(size_t capacity) {
    }

    ~FixedDlxNodeList() {
    }

    size_t size() const { return Capacity; }
    size_t capacity() const { return this_type::kCapacity; }
};

class DancingLinks {
public:
    static const size_t Rows = Sudoku::Rows;
    static const size_t Cols = Sudoku::Cols;
    static const size_t Palaces = Sudoku::Palaces;
    static const size_t Numbers = Sudoku::Numbers;

    static const size_t TotalSize = Sudoku::TotalSize;
    static const size_t TotalSize2 = Sudoku::TotalSize2;

    static size_t init_counter;
    static size_t num_guesses;
    static size_t num_unique_candidate;
    static size_t num_early_return;

private:
    struct col_info_t {
        uint8_t size;
        uint8_t enable;
    };
#if 0
    DlxNodeList         list_;
#else
    FixedDlxNodeList<Sudoku::TotalSize * 4 + 1>
                        list_;
#endif

    SmallBitMatrix2<9, 9>  bit_rows;        // [row][num]
    SmallBitMatrix2<9, 9>  bit_cols;        // [col][num]
    SmallBitMatrix2<9, 9>  bit_palaces;     // [palace][num]

#if defined(__SSE_4_1__)
    alignas(64) col_info_t col_info_[Sudoku::TotalConditions + 1];
#else
    alignas(32) uint8_t col_size_[Sudoku::TotalConditions + 1];
    alignas(32) uint8_t col_enable_[Sudoku::TotalConditions + 1];
#endif
    int                 max_col_;
    int                 last_idx_;
    std::vector<int>    answer_;
    size_t              empties_;

    unsigned short      col_index_[Sudoku::TotalConditions + 1];

    unsigned short rows_[TotalSize + 1];
    unsigned short cols_[TotalSize + 1];
    unsigned short numbers_[TotalSize + 1];

    std::vector<std::vector<int>> answers_;

public:
    DancingLinks(size_t nodes)
        : list_(nodes), max_col_(0), last_idx_(0), empties_(0) {
    }

    ~DancingLinks() {}

    bool is_empty() const { return (list_.next[0] == 0); }

    int cols() const { return (int)Sudoku::TotalConditions; }

    static size_t get_init_counter() { return DancingLinks::init_counter; }
    static size_t get_num_guesses() { return DancingLinks::num_guesses; }
    static size_t get_num_unique_candidate() { return DancingLinks::num_unique_candidate; }
    static size_t get_num_early_return() { return DancingLinks::num_early_return; }

    static size_t get_search_counter() {
        return (DancingLinks::num_guesses + DancingLinks::num_unique_candidate + DancingLinks::num_early_return);
    }

    static double get_guess_percent() {
        return calc_percent(DancingLinks::num_guesses, DancingLinks::get_search_counter());
    }

    static double get_early_return_percent() {
        return calc_percent(DancingLinks::num_early_return, DancingLinks::get_search_counter());
    }

    static double get_unique_candidate_percent() {
        return calc_percent(DancingLinks::num_unique_candidate, DancingLinks::get_search_counter());
    }

private:
#if defined(__SSE_4_1__)
    int get_min_column(int & out_min_col) const {
        int first = list_.next[0];
        assert(first != 0);
        int min_col = col_info_[first].size;
        assert(min_col >= 0);
        if (min_col <= 1) {
            out_min_col = min_col;
            return first;
        }
        int min_col_index = first;
        for (int i = list_.next[first]; i != 0; i = list_.next[i]) {
            int col_size = col_info_[i].size;
            if (col_size < min_col) {
                assert(col_size >= 0);
                if (col_size <= 1) {
                    if (col_size == 0) {
                        return 0;
                    }
                    else {
                        out_min_col = 1;
                        return i;
                    }
                }
                min_col = col_size;
                min_col_index = i;
            }
        }
        out_min_col = min_col;
        return min_col_index;
    }

    int get_min_column_simd(int & out_min_col) {
        int min_col = 256;
        int min_col_index = 0;
        int index_base = 0;

        const char * pinfo     = (const char *)&col_info_[0];
        const char * pinfo_end = (const char *)&col_info_[this->max_col_];
        while ((pinfo_end - pinfo) >= 64) {
            __m128i xmm0 = _mm_load_si128((const __m128i *)(pinfo + 0));
            __m128i xmm1 = _mm_load_si128((const __m128i *)(pinfo + 16));
            __m128i xmm2 = _mm_load_si128((const __m128i *)(pinfo + 32));
            __m128i xmm3 = _mm_load_si128((const __m128i *)(pinfo + 48));

            __m128i xmm4 = _mm_minpos_epu16(xmm0);
            __m128i xmm5 = _mm_minpos_epu16(xmm1);
            __m128i xmm6 = _mm_minpos_epu16(xmm2);
            __m128i xmm7 = _mm_minpos_epu16(xmm3);

            __m128i xmm5_ = _mm_slli_epi64(xmm5, 32);
            __m128i xmm7_ = _mm_slli_epi64(xmm7, 32);
            __m128i xmm4_5  = _mm_blend_epi16(xmm4, xmm5_, 0b00001100);
            __m128i xmm6_7  = _mm_blend_epi16(xmm6, xmm7_, 0b00001100);
            __m128i xmm6_7_ = _mm_slli_si128(xmm6_7, 8);
            __m128i comb_0  = _mm_or_si128(xmm4_5, xmm6_7_);
            __m128i __col_index = comb_0;

            __m128i kColSizeMask = _mm_set1_epi32((int)0xFFFF0000L);
            __m128i __col_size = _mm_or_si128(comb_0, kColSizeMask);
            __m128i __min_col_size = _mm_minpos_epu16(__col_size);

            uint32_t min_col_size32 = (uint32_t)_mm_cvtsi128_si32(__min_col_size);
            int min_col_size = (int)(min_col_size32 & 0x0000FFFFULL);
            if (min_col_size < min_col) {
                min_col = min_col_size;

                uint32_t min_block_index16 = min_col_size32 >> 17U;
                __m128i __min_col_index_sr15 = _mm_srli_epi64(__min_col_size, 15);

                __m128i __col_index_sr16 = _mm_srli_epi32(__col_index, 16);

                // SSSE3
                __m128i __min_col_index = _mm_shuffle_epi8(__col_index_sr16, __min_col_index_sr15);
                uint32_t min_col_index32 = (uint32_t)_mm_cvtsi128_si32(__min_col_index);
                int min_col_offset = (int)(min_col_index32 & 0x000000FFUL);
                min_col_index = index_base + min_block_index16 * 8 + min_col_offset;

                if (min_col == 0) {
                    return 0;
                }
#if 0
                else if (min_col == 1) {
                    out_min_col = 1;
                    return min_col_index;
                }
#endif
            }
            index_base += 32;
            pinfo += 64;
        }

        if ((pinfo_end - pinfo) >= 32) {
            __m128i xmm0 = _mm_load_si128((const __m128i *)(pinfo + 0));
            __m128i xmm1 = _mm_load_si128((const __m128i *)(pinfo + 16));

            __m128i xmm2 = _mm_minpos_epu16(xmm0);
            __m128i xmm3 = _mm_minpos_epu16(xmm1);

            __m128i xmm3_ = _mm_slli_epi64(xmm3, 32);
            __m128i comb_0 = _mm_blend_epi16(xmm2, xmm3_, 0b00001100);
            __m128i __col_index = comb_0;

            __m128i kColSizeMask = _mm_set_epi32(0xFFFFFFFFL, 0xFFFFFFFFL, 0xFFFF0000L, 0xFFFF0000L);
            __m128i __col_size = _mm_or_si128(comb_0, kColSizeMask);
            __m128i __min_col_size = _mm_minpos_epu16(__col_size);

            uint32_t min_col_size32 = (uint32_t)_mm_cvtsi128_si32(__min_col_size);
            int min_col_size = (int)(min_col_size32 & 0x0000FFFFULL);
            if (min_col_size < min_col) {
                min_col = min_col_size;

                uint32_t min_block_index16 = min_col_size32 >> 17U;
                __m128i __min_col_index_sr15 = _mm_srli_epi64(__min_col_size, 15);

                __m128i __col_index_sr16 = _mm_srli_epi32(__col_index, 16);

                // SSSE3
                __m128i __min_col_index = _mm_shuffle_epi8(__col_index_sr16, __min_col_index_sr15);
                uint32_t min_col_index32 = (uint32_t)_mm_cvtsi128_si32(__min_col_index);
                int min_col_offset = (int)(min_col_index32 & 0x000000FFUL);
                min_col_index = index_base + min_block_index16 * 8 + min_col_offset;

                if (min_col == 0) {
                    return 0;
                }
#if 0
                else if (min_col == 1) {
                    out_min_col = 1;
                    return min_col_index;
                }
#endif
            }
            index_base += 16;
            pinfo += 32;
        }

        if ((pinfo_end - pinfo) >= 16) {
            __m128i xmm0 = _mm_load_si128((const __m128i *)(pinfo + 0));
            __m128i __min_col_size = _mm_minpos_epu16(xmm0);

            uint32_t min_col_size32 = (uint32_t)_mm_cvtsi128_si32(__min_col_size);
            int min_col_size = (int)(min_col_size32 & 0x0000FFFFULL);
            if (min_col_size < min_col) {
                min_col = min_col_size;

                uint32_t min_col_offset = min_col_size32 >> 17U;
                min_col_index = index_base + min_col_offset;

                if (min_col == 0) {
                    return 0;
                }
#if 0
                else if (min_col == 1) {
                    out_min_col = 1;
                    return min_col_index;
                }
#endif
            }
            index_base += 8;
            pinfo += 16;
        }

        // Last remain items (less than 8 items)
        while (pinfo < pinfo_end) {
            col_info_t * pcol_info = (col_info_t *)pinfo;
            if (pcol_info->enable == 0) {
                int col_size = pcol_info->size;
                if (col_size < min_col) {
                    if (col_size == 0) {
                        return 0;
                    }
                    min_col = col_size;
                    min_col_index = index_base;
                }
            }
            index_base++;
            pinfo += 2;
        }

        out_min_col = min_col;
        return min_col_index;
    }

#else // !__SSE_4_1__

    int get_min_column(int & out_min_col) const {
        int first = list_.next[0];
        assert(first != 0);
        int min_col = col_size_[first];
        assert(min_col >= 0);
        if (min_col <= 1) {
            out_min_col = min_col;
            return first;
        }
        int min_col_index = first;
        for (int i = list_.next[first]; i != 0; i = list_.next[i]) {
            int col_size = col_size_[i];
            if (col_size < min_col) {
                assert(col_size >= 0);
                if (col_size <= 1) {
                    if (col_size == 0) {
                        return 0;
                    }
                    else {
                        out_min_col = 1;
                        return i;
                    }
                }
                min_col = col_size;
                min_col_index = i;
            }
        }
        out_min_col = min_col;
        return min_col_index;
    }

    int get_min_column_simd(int & out_min_col) {
        return 0;
    }

#endif // __SSE_4_1__

    std::bitset<9> getUsable(size_t row, size_t col) {
        size_t palace = row / 3 * 3 + col / 3;
        // size_t palace = tables.roundTo3[row] + tables.div3[col];
        return ~(this->bit_rows[row] | this->bit_cols[col] | this->bit_palaces[palace]);
    }

    std::bitset<9> getUsable(size_t row, size_t col, size_t palace) {
        return ~(this->bit_rows[row] | this->bit_cols[col] | this->bit_palaces[palace]);
    }

    void fillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        // size_t palace = tables.roundTo3[row] + tables.div3[col];
        this->bit_rows[row].set(num);
        this->bit_cols[col].set(num);
        this->bit_palaces[palace].set(num);
    }

public:
    int filter_unused_cols(char board[Sudoku::BoardSize]) {
        std::memset(&this->col_index_[0], 0, sizeof(this->col_index_));

        size_t pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            size_t palace_row = row / 3 * 3;
            for (size_t col = 0; col < Cols; col++) {
                unsigned char val = board[pos];
                if (val != '.') {
                    size_t num = val - '1';
                    this->col_index_[0      + pos           + 1] = 0xFFFF;
                    this->col_index_[81 * 1 + row * 9 + num + 1] = 0xFFFF;
                    this->col_index_[81 * 2 + col * 9 + num + 1] = 0xFFFF;
                    size_t palace = palace_row + col / 3;
                    // size_t palace_x_9 = tables.palace_x_9[pos];
                    this->col_index_[81 * 3 + palace * 9 + num + 1] = 0xFFFF;
                }
                pos++;
            }
        }

        size_t index = 1;
        for (size_t i = 1; i < (Sudoku::TotalConditions + 1); i++) {
            if (this->col_index_[i] == 0) {
                this->col_index_[i] = (unsigned short)index;
                index++;
            }
        }
        return (int)(index - 1);
    }

    void init(char board[Sudoku::BoardSize]) {
        int cols = this->filter_unused_cols(board);
        for (int col = 0; col <= cols; col++) {
            list_.prev[col] = col - 1;
            list_.next[col] = col + 1;
            list_.up[col] = col;
            list_.down[col] = col;
        }
        list_.prev[0] = cols;
        list_.next[cols] = 0;

        this->max_col_ = cols + 1;
        this->last_idx_ = cols + 1;

#if defined(__SSE_4_1__)
#if 1
        std::memset((void *)&col_info_[0], 0, (cols + 1) * sizeof(col_info_t));
#else
        for (int i = 0; i <= cols; i++) {
            col_info_[i].size = 0;
            col_info_[i].enable = 0;
        }
#endif
        col_info_[0].size = 255;
        //col_info_[0].enable = 0;
#else // !__SSE_4_1__
#if 1
        std::memset((void *)&col_size_[0], 0, (cols + 1) * sizeof(uint8_t));
        std::memset((void *)&col_enable_[0], 0x0F, (cols + 1) * sizeof(uint8_t));
#else
        for (int i = 0; i <= cols; i++) {
            col_size_[i] = 0;
            col_enable_[i] = 0x0F;
        }
#endif
        col_size_[0] = 255;
        col_enable_[0] = 0xFF;
#endif // __SSE_4_1__

        this->bit_rows.reset();
        this->bit_cols.reset();
        this->bit_palaces.reset();

        this->answer_.clear();
        this->answer_.reserve(81);
        if (kSearchMode > SEARCH_MODE_ONE_ANSWER) {
            this->answers_.clear();
        }
        init_counter = 0;
        num_guesses = 0;
        num_unique_candidate = 0;
        num_early_return = 0;
    }

    void build(char board[Sudoku::BoardSize]) {
        size_t empties = 0;
        size_t pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            for (size_t col = 0; col < Cols; col++) {
                unsigned char val = board[pos++];
                if (val == '.') {
                    empties++;
                }
                else {
                    size_t num = val - '1';
                    this->fillNum(row, col, num);
                }
            }
        }

        this->empties_ = empties;

        // maxRows = filled * 1 + empties * 9;
        //         = (9 * 9 - empties) * 1 + empties * 9;
        //         = (9 * 9) + empties * 8;
        size_t filled = Rows * Cols - empties;
        size_t maxRows = filled * 1 +  empties * Numbers;        

        int row_idx = 1;

        pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            size_t palace_row = row / 3 * 3;
            for (size_t col = 0; col < Cols; col++) {
                unsigned char val = board[pos];
                if (val == '.') {
                    size_t palace = palace_row + col / 3;
                    // size_t palace = tables.palace[pos];
                    // size_t palace_x_9 = tables.palace_x_9[pos];
                    std::bitset<9> numsUsable = getUsable(row, col, palace);
                    for (size_t number = 0; number < Numbers; number++) {
                        if (numsUsable.test(number)) {
                            int head = last_idx_;
                            int index = last_idx_;

                            this->insert(index + 0, row_idx, (int)(81 * 0 + pos                 + 1));
                            this->insert(index + 1, row_idx, (int)(81 * 1 + row * 9    + number + 1));
                            this->insert(index + 2, row_idx, (int)(81 * 2 + col * 9    + number + 1));
                            this->insert(index + 3, row_idx, (int)(81 * 3 + palace * 9 + number + 1));

                            this->rows_[row_idx] = (unsigned short)row;
                            this->cols_[row_idx] = (unsigned short)col;
                            this->numbers_[row_idx] = (unsigned short)number;
                            index += 4;
                            row_idx++;

                            list_.next[index - 1] = head;
                            list_.prev[head] = index - 1;
                            last_idx_ = index;
                        }
                    }
                }               
                pos++;
            }
        }
        assert(row_idx <= (maxRows + 1));
    }

    void insert(int index, int row, int col) {
        int save_col = col;
        col = this->col_index_[col];
        assert(col != 0xFFFF);
        list_.prev[index] = index - 1;
        list_.next[index] = index + 1;
        list_.up[index] = list_.up[col];
        list_.down[index] = col;
        list_.row[index] = row;
        list_.col[index] = col;
#if 1
        list_.down[list_.up[index]] = index;
        list_.up[col] = index;
#else
        list_.down[list_.up[index]] = index;
        list_.up[list_.down[index]] = index;
#endif
        this->inc_col_size(col);
    }

#if defined(__SSE_4_1__)

    inline void set_col_enable(int index) {
        this->col_info_[index].enable = 0x00;
    }

    inline void set_col_disable(int index) {
        this->col_info_[index].enable = 0xFF;
    }

    inline uint8_t get_col_size(int index) {
        return this->col_info_[index].size;
    }

    inline void inc_col_size(int index) {
        this->col_info_[index].size++;
    }

    inline void dec_col_size(int index) {
        this->col_info_[index].size--;
    }

#else // !__SSE_4_1__

    inline void set_col_enable(int index) {
        this->col_enable_[index] = 0x0F;
    }

    inline void set_col_disable(int index) {
        this->col_enable_[index] = 0xFF;
    }

    inline uint8_t get_col_size(int index) {
        return this->col_size_[index];
    }

    inline void inc_col_size(int index) {
        this->col_size_[index]++;
    }

    inline void dec_col_size(int index) {
        this->col_size_[index]--;
    }

#endif // __SSE_4_1__

    void remove(int index) {
        assert(index > 0);
        int prev = list_.prev[index];
        int next = list_.next[index];
        list_.next[prev] = next;
        list_.prev[next] = prev;

        this->set_col_disable(index);

        for (int row = list_.down[index]; row != index; row = list_.down[row]) {
            for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                int up = list_.up[col];
                int down = list_.down[col];
                list_.down[up] = down;
                list_.up[down] = up;

                uint16_t col_index = list_.col[col];
                assert(this->get_col_size(col_index) > 0);
                this->dec_col_size(col_index);
                this->set_col_enable(col_index);
            }
        }
    }

    void restore(int index) {
        assert(index > 0);
        int next = list_.next[index];
        int prev = list_.prev[index];
        list_.prev[next] = index;
        list_.next[prev] = index;

        this->set_col_enable(index);

        for (int row = list_.up[index]; row != index; row = list_.up[row]) {
            for (int col = list_.prev[row]; col != row; col = list_.prev[col]) {
                int down = list_.down[col];
                int up = list_.up[col];
                list_.up[down] = col;
                list_.down[up] = col;

                uint16_t col_index = list_.col[col];
                this->inc_col_size(col_index);
                this->set_col_enable(col_index);
            }
        }
    }

    bool search(size_t empties) {
        if (this->is_empty()) {
            if (kSearchMode > SearchMode::OneAnswer) {
                this->answers_.push_back(this->answer_);
                if (kSearchMode == SearchMode::MoreThanOneAnswer) {
                    if (this->answers_.size() > 1)
                        return true;
                }
            }
            else {
                return true;
            }
        }
        
        int min_col;
        int index;
        if (empties > 8)
            index = get_min_column_simd(min_col);
        else
            index = get_min_column(min_col);
        if (index > 0) {
            if (min_col == 1)
                num_unique_candidate++;
            else
                num_guesses++;
            this->remove(index);
            for (int row = list_.down[index]; row != index; row = list_.down[row]) {
                this->answer_.push_back(list_.row[row]);
                for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                    this->remove(list_.col[col]);
                }

                if (this->search(empties - 1)) {
                    if (kSearchMode == SearchMode::OneAnswer) {
                        return true;
                    }
                    else if (kSearchMode == SearchMode::MoreThanOneAnswer) {
                        if (this->answers_.size() > 1)
                            return true;
                    }
                }

                for (int col = list_.prev[row]; col != row; col = list_.prev[col]) {
                    this->restore(list_.col[col]);
                }
                this->answer_.pop_back();
            }
            this->restore(index);
        }
        else {
            num_early_return++;
        }

        return false;
    }

    bool solve() {
#if 0
RETRY_UNIQUE_CANDIDATE:
        for (int index = list_.next[0]; index != 0; index = list_.next[index]) {
            size_t col_size = col_size_[index];
            if (col_size == 1) {
                assert(index > 0);

                this->remove(index);
                for (int row = list_.down[index]; row != index; row = list_.down[row]) {
                    this->answer_.push_back(list_.row[row]);
                    for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                        this->remove(list_.col[col]);
                    }
                }

                init_counter++;
                goto RETRY_UNIQUE_CANDIDATE;
            }
            else if (col_size == 0) {
                return false;
            }
        }
#endif
        return this->search(this->empties_);
    }

    void display_answer(char board[Sudoku::BoardSize]) {
        for (auto idx : this->answer_) {
            if (idx > 0) {
                board[this->rows_[idx] * Rows + this->cols_[idx]] = (char)this->numbers_[idx] + '1';
            }
        }

        Sudoku::display_board(board);
    }

    void display_answers(char board[Sudoku::BoardSize]) {
        printf("Total answers: %d\n\n", (int)this->answers_.size());
        int i = 0;
        for (auto answer : this->answers_) {
            Sudoku::clear_board(board);
            for (auto idx : answer) {
                if (idx > 0) {
                    board[this->rows_[idx] * Rows + this->cols_[idx]] = (char)this->numbers_[idx] + '1';
                }
            }
            Sudoku::display_board(board, false, i);
            i++;
            if (i > 100)
                break;
        }
    }
};

size_t DancingLinks::init_counter = 0;
size_t DancingLinks::num_guesses = 0;
size_t DancingLinks::num_unique_candidate = 0;
size_t DancingLinks::num_early_return = 0;

class Solver {
public:
    typedef DancingLinks algorithm;

private:
    DancingLinks solver_;

public:
    Solver() : solver_(Sudoku::TotalSize * 4 + 1) {
    }
    ~Solver() {}

public:
    bool solve(char board[Sudoku::BoardSize],
               double & elapsed_time,
               bool verbose = true) {
        if (verbose) {
            Sudoku::display_board(board, true);
        }

        jtest::StopWatch sw;
        sw.start();

        solver_.init(board);
        solver_.build(board);
        bool success = solver_.solve();

        sw.stop();
        elapsed_time = sw.getElapsedMillisec();

        if (verbose) {
            if (kSearchMode > SearchMode::OneAnswer)
                solver_.display_answers(board);
            else
                solver_.display_answer(board);
            printf("elapsed time: %0.3f ms, init_counter: %" PRIuPTR ", recur_counter: %" PRIuPTR "\n\n"
                   "num_guesses: %" PRIuPTR ", num_early_return: %" PRIuPTR ", unique_candidate: %" PRIuPTR "\n"
                   "guess %% = %0.1f %%, early_return %% = %0.1f %%, unique_candidate %% = %0.1f %%\n\n",
                   elapsed_time, DancingLinks::get_init_counter(),
                   DancingLinks::get_search_counter(),
                   DancingLinks::get_num_guesses(),
                   DancingLinks::get_num_early_return(),
                   DancingLinks::get_num_unique_candidate(),
                   DancingLinks::get_guess_percent(),
                   DancingLinks::get_early_return_percent(),
                   DancingLinks::get_unique_candidate_percent());
        }

        return success;
    }
};

} // namespace v3
} // namespace jmSudoku

#endif // JM_SUDOKU_SOLVER_V3_H