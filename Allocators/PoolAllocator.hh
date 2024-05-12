#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H
#include <stddef.h>

#include "resources/Allocator.hh"
class PoolAllocator : public Allocator {
   public:
    PoolAllocator();
    ~PoolAllocator();
    char* allocate(size_t size) override;
    void deallocate(void*) override;

   private:
    struct Buffer;
    Buffer* buffer_list_head;
    size_t space, top;
    void createNewBuffer(size_t size);
};
#endif  // POOLALLOCATOR_H