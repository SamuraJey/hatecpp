#pragma once
#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H
#include <stddef.h>

#include "Allocator.hh"
class PoolAllocator : public Allocator {
    // struct Buffer {
    //     Buffer* prev;
    //     size_t current;
    //     size_t size;
    // };

    // Buffer* some_buffer;

   public:
    PoolAllocator();
    ~PoolAllocator();
    char* allocate(size_t size) override;
    void deallocate(void*) override;

   private:
    // декларация структуры в класе
    // реализация прописыватеся в cpp через указывания неймспейса класса, так же как и остальные поля
    struct Buffer;
    Buffer* buffer_list_head;
    void createNewBuffer(size_t size);
};
#endif  // POOLALLOCATOR_H