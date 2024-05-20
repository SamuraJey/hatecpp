#ifndef DESCRIPTOR_ALLOCATOR_H
#define DESCRIPTOR_ALLOCATOR_H

#include "resources/Allocator.hh"

/**
 * @class DescriptorAllocator
 * @brief A class that represents a descriptor allocator.
 * 
 * The DescriptorAllocator class is a subclass of the Allocator class. It provides functionality for allocating and deallocating memory blocks.
 * Using linked list of free blocks, it allocates memory blocks of the requested size using the first-fit strategy and descriptors at the sides of the block.
 */
class DescriptorAllocator : public Allocator {
   public:
    DescriptorAllocator();
    ~DescriptorAllocator();
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
