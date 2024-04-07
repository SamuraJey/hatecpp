#ifndef DESCRIPTOR_ALLOCATOR_H
#define DESCRIPTOR_ALLOCATOR_H

#include "Allocator.hh"

class DescriptorAllocator : public Allocator {
   public:
    DescriptorAllocator();
    ~DescriptorAllocator();
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

    void check_memory();

   private:
    struct BlockHeader;
    // Alias for Allocated header size
    static const size_t alloc_header;
    static const size_t free_header;
    char* const buffer;
    char* const endBuffer;
    BlockHeader* root;
    // add block to free list next to root
    void add_free(BlockHeader* newBlock);
    // pop from free list
    void pop_free(BlockHeader* cur);
    // spaw block from free list with another block
    void list_swap(BlockHeader* from, BlockHeader* to);
};
#endif  // DESCRIPTOR_ALLOCATOR_H
