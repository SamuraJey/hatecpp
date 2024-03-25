// класс - контрольная группа, для сравнения скорости.

#ifndef REFERENCEALLOCACOR_H
#define REFERENCEALLOCACOR_H

#include <stdlib.h>

#include "Allocator.hh"
class ReferenceAllocator : public Allocator {
   public:
    inline char* allocate(size_t size) override {
        return (char*)malloc(size);
    };

    inline void deallocate(void* ptr) override {
        free(ptr);
    };
};
#endif