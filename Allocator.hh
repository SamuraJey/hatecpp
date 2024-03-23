#pragma once
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
class Allocator {
   public:
    virtual char* allocate(size_t count) = 0;
    virtual void deallocate(void* p) = 0;
};

#endif