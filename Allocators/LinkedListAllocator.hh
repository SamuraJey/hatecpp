#ifndef LINKEDLISTALLOCATOR_H
#define LINKEDLISTALLOCATOR_H

#include "resources/Allocator.hh"
#include "resources/constants.hh"

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