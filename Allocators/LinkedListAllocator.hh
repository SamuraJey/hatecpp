#ifndef LINKEDLISTALLOCATOR_H
#define LINKEDLISTALLOCATOR_H

#include "resources/Allocator.hh"
#include "resources/constants.hh"

/**
 * @class LinkedListAllocator
 * @brief A memory allocator that uses a linked list data structure to manage memory blocks.
 * 
 * The LinkedListAllocator class is a concrete implementation of the Allocator interface.
 * It provides methods to allocate and deallocate memory blocks using a linked list data structure.
 * 
 * The allocator maintains a buffer of memory and uses a linked list of block headers to keep track of allocated and free blocks.
 * Each block header contains information about the size and status (allocated or free) of the block.
 */
class LinkedListAllocator : public Allocator {
   public:
    LinkedListAllocator();
    ~LinkedListAllocator();
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

   private:
    struct BlockHeader;
    char* const buffer;
    BlockHeader* root;
    // Alias for Free header size
    static const size_t free_header;
    // Alias for Allocated header size
    static const size_t alloc_header;
    void mark_used(BlockHeader* block);

#if DEBUG
    // сумма байт всех запросов
    size_t bytes_allocated;
    // сумма занятых байт
    size_t bytes_used;
    size_t max_bytes_used;
    size_t block_counter;
    size_t max_block_count;
    size_t block_size_distribution[LINKED_BUFFER_SIZE + 1] = {0};
#endif
};

#endif  // LINKEDLISTALLOCATOR_H