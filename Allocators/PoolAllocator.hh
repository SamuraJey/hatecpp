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
    void createNewBuffer(size_t size);
    char* allocate(size_t size) override;
    void deallocate(void*) override;
};
#endif  // POOLALLOCATOR_H