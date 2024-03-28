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
    // Декларация структуры в классе
    // Реализация прописыватеся в cpp через неймспейса класса, так же как и остальные поля
    struct Buffer;
    Buffer* buffer_list_head;  // Ссылка на список буферов - поле. Для каждого экземпляра аллокатора она своя
    void createNewBuffer(size_t size);
};
#endif  // POOLALLOCATOR_H