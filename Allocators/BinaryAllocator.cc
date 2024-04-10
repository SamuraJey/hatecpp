#include <bit>
#include <bitset>
#include <new>

#include "../constants.hh"
#include "Allocator.hh"
#include "immintrin.h"

constexpr bool isPowOf2(size_t x) {
    return x && !(x & (x - 1));  // true for x = 2^n, n in [0,1,.,63]
}

static_assert(sizeof(void*) <= BINARY_MIN_BLOCK_SIZE && BINARY_MIN_BLOCK_SIZE <= BINARY_BUFFER_SIZE);
static_assert(std::has_single_bit(BINARY_BUFFER_SIZE));
static_assert(std::has_single_bit(BINARY_MIN_BLOCK_SIZE));

static inline uint64_t minLevel(size_t size) noexcept {
    // static делает переменную глобальной, но расположение в функции огранничевает видимость этой функцией
    // __builtin_clzll - интринзик, обёртка над ассемблерной инструкцией в виде функции, предоставляемая компилятором
    // считает количество незначащих нулей в двоичной записи числа
    static constexpr uint64_t min_pow = 62u - __builtin_clzll(BINARY_MIN_BLOCK_SIZE);
    uint64_t log2;
    // inline assembly with bsr instruction (Bit Scan Reverse)
    // find index of most significant (right most) set (1) bit in an integer
    asm("bsr %1, %0" : "=r"(log2) : "r"((size - 1) >> min_pow));
    return log2;
}

static inline uint64_t maxLevel(size_t ofset) noexcept {
    __builtin_crzll()
}

class BinaryAllocator : public Allocator {
   public:
    BinaryAllocator();
    ~BinaryAllocator();
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

   public:
    struct ListNode;
    char* const buffer;
    ListNode* listArr[];
    static inline uint64_t calcPow(size_t size) {
        uint64_t log;
        // inline assembly with bsr instruction (Bit Scan Reverse)
        // find index of most significant (rightest) set (1) bit in an integer
        asm("bsr %1, %0" : "=r"(log) : "r"(size - 1));
        return log;
    }
    // std::bit_width()
    // std::bitset
};

char* BinaryAllocator::allocate(size_t size) {
    char* dummy = new char('a');
    return dummy;
}

void BinaryAllocator::deallocate(void* ptr) {
    return;
}