#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H
#include <stddef.h>

#include "Allocator.hh"
class PoolAllocator : public Allocator {
   public:
    PoolAllocator();
    ~PoolAllocator();
    char* allocate(size_t size) override;
    void deallocate(void*) override;

   private:
    struct Buffer;
    Buffer* buffer_list_head;  // Ссылка на список буферов - поле. Для каждого экземпляра аллокатора она своя
    void createNewBuffer(size_t size);
};
#endif  // POOLALLOCATOR_H