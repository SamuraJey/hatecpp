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
static_assert(isPowOf2(BINARY_BUFFER_SIZE));
static_assert(isPowOf2(BINARY_MIN_BLOCK_SIZE));

class BinaryAllocator : public Allocator {
   public:
    BinaryAllocator();
    ~BinaryAllocator();
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

   private:
    struct ListNode;
    char* const buffer;
    ListNode* listArr[];
    // std::bit_width()
    // std::bitset
};