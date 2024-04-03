#ifndef DESCRIPTOR_ALLOCATOR_H
#define DESCRIPTOR_ALLOCATOR_H

#include "Allocator.hh"

class DescriptorAllocator : public Allocator {
   private:
    struct BlockHeader;
    struct Descriptor;
    char* buffer;
    BlockHeader* root;
    char* endBuffer;

   public:
    DescriptorAllocator();

    void remove(BlockHeader* cur);

    void insert(BlockHeader* newBlock);

    char* allocate(size_t size) override;

    void deallocate(void* ptr) override;

    void check_memory();
};
#endif  // DESCRIPTOR_ALLOCATOR_H
