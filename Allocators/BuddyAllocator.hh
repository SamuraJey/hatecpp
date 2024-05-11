#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include "../constants.hh"
#include "Allocator.hh"
#include "List.tpp"

class BuddyAllocator : public Allocator {
   private:
    struct BlockHeader;
    List<BlockHeader> levelLists[BADDY_MAX_LEVEL + 1];
    char* const buffer;

   public:
    BuddyAllocator();
    ~BuddyAllocator();

    char* allocate(size_t size);
    void deallocate(void* ptr);

    void check_memory();

   private:
    // scary exprassion wrapper functions
    size_t Block_idx(BlockHeader* block_ptr, size_t block_level) noexcept;
    BlockHeader* Block_ptr(size_t block_idx, size_t block_level) noexcept;
};

#endif  // !BUDDYALLOCATOR_H
