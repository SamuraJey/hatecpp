#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include <cstdint>

#include "resources/Allocator.hh"
#include "resources/List.tpp"
#include "resources/constants.hh"

class BuddyAllocator : public Allocator {
   private:
    static_assert(BUDDY_MAX_LEVEL < 64);
    struct BlockHeader;
    // Массив списков для свободных блоков каждого уровня
    List<BlockHeader> levelLists[BUDDY_MAX_LEVEL + 1];
    // Битовая маска показывающая не пустоту списка соответствующего уровня
    uint64_t listsMask;
    char* const buffer;

   public:
    BuddyAllocator();
    ~BuddyAllocator();

    char* allocate(size_t size);
    void deallocate(void* ptr);

    void check_memory();

   private:
    // scary expression wrapper functions
    size_t Block_idx(BlockHeader* block_ptr, size_t block_level) noexcept;
    BlockHeader* Block_ptr(size_t block_idx, size_t block_level) noexcept;
};

#endif  // !BUDDYALLOCATOR_H
