#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include <cstdint>

#include "resources/Allocator.hh"
#include "resources/List.tpp"
#include "resources/constants.hh"

/**
 * @class BuddyAllocator
 * @brief A class that implements a buddy allocator for managing memory allocation and deallocation.
 * 
 * The BuddyAllocator class is a derived class of the Allocator base class. It provides functionality
 * for allocating and deallocating memory using the buddy(binary) allocation algorithm. It
 * divides the memory into blocks sizes in power of 2 and maintains a list of free blocks at each level (power of 2).
 * 
 */
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
    inline size_t Block_idx(BlockHeader* block_ptr, size_t block_level) noexcept;
    inline BlockHeader* Block_ptr(size_t block_idx, size_t block_level) noexcept;
};

#endif  // !BUDDYALLOCATOR_H
