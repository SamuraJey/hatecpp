#ifndef DESCRIPTOR_ALLOCATOR_H
#define DESCRIPTOR_ALLOCATOR_H

#include "Allocator.hh"

class DescriptorAllocator : public Allocator {
   public:
    DescriptorAllocator();
    // Add destructor for memory deallocation
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

    void check_memory();

   private:
    struct BlockHeader;
    struct Descriptor;
    // Менять ссылку на буфер мы не захотим.
    // С пометкой константы писать логику безопастнее.
    char* const buffer;
    char* const endBuffer;
    BlockHeader* root;
    // mark block used and remove from free list
    void mark_used(BlockHeader* cur);
    // mark block free and add it to free list before root
    void mark_free(BlockHeader* newBlock);
    // spaw block from free list with another block
    void list_swap(BlockHeader* from, BlockHeader* to);
};
#endif  // DESCRIPTOR_ALLOCATOR_H
