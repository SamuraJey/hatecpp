// Класс использующий стандартный аллокатор malloc, используется как отправная точка с котрой мы будем сравнивать наши самописные аллокаторы.

#ifndef REFERENCEALLOCACOR_H
#define REFERENCEALLOCACOR_H

#include <stdlib.h>

#include "resources/Allocator.hh"
/**
 * @class ReferenceAllocator
 * @brief A class that represents a reference allocator.
 * 
 * The ReferenceAllocator class is a concrete implementation of the Allocator interface.
 * It provides methods to allocate and deallocate memory using the standard library functions malloc and free.
 */
class ReferenceAllocator : public Allocator {
   public:
    /**
     * @brief Allocates a block of memory of the given size.
     * @param size The size of the memory block to allocate.
     * @return A pointer to the allocated memory block.
     */
    inline char* allocate(size_t size) override {
        return static_cast<char*>(malloc(size));
    };

    /**
     * @brief Deallocates a previously allocated memory block.
     * @param ptr A pointer to the memory block to deallocate.
     */
    inline void deallocate(void* ptr) override {
        free(ptr);
    };
};
#endif  // REFERENCEALLOCACOR_H