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
    // Maybe add aliases for sizes, but it isn't esential
    void mark_used(BlockHeader* cur);
    void mark_unused(BlockHeader* newBlock);
};
#endif  // DESCRIPTOR_ALLOCATOR_H
