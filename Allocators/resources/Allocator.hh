#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef>
/**
 * @class Allocator
 * @brief Abstract base class for memory allocators.
 *
 * The `Allocator` class provides an interface for allocating and deallocating memory.
 * Derived classes must implement the `allocate` and `deallocate` methods.
 */
class Allocator {
   public:
    /**
     * @brief Allocates a block of memory of the specified size.
     * @param count The number of bytes to allocate.
     * @return A pointer to the allocated memory block.
     */
    virtual char* allocate(size_t count) = 0;

    /**
     * @brief Deallocates a previously allocated memory block.
     * @param p A pointer to the memory block to deallocate.
     */
    virtual void deallocate(void* p) = 0;
};

#endif // ALLOCATOR_H