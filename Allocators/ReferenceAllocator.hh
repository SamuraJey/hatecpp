// Класс использующий стандартный аллокатор malloc, используется как отправная точка с котрой мы будем сравнивать наши самописные аллокаторы.

#ifndef REFERENCEALLOCACOR_H
#define REFERENCEALLOCACOR_H

#include <stdlib.h>

#include "Allocator.hh"
class ReferenceAllocator : public Allocator {
   public:
    inline char* allocate(size_t size) override {
        return static_cast<char*>(malloc(size));
    };

    inline void deallocate(void* ptr) override {
        free(ptr);
    };
};
#endif  // REFERENCEALLOCACOR_H