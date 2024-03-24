#pragma once
#ifndef LINKEDLISTALLOCATOR_H
#define LINKEDLISTALLOCATOR_H

#include "../constants.hh"
#include "Allocator.hh"

class LinkedListAllocator : public Allocator {
   public:
    LinkedListAllocator();
    ~LinkedListAllocator();
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;

   private:
    char* buffer;
    struct BlockHeader;
    BlockHeader* root;
    // Алиасам в глобальном блоке видимости точно не место
    // Alias for Free header size
    static const size_t F_hrader;
    // Alias for Allocated header size
    static const size_t A_header;
    void remove_from_list(BlockHeader* rem);
    // И к сожалению это тоже нужно сделать частью класса

#if DEBUG
    // сумма байт всех запросов
    size_t bytes_allocated;
    // сумма занятых байт
    size_t bytes_used;
    size_t block_counter;
    size_t max_block_count;
    size_t block_size_distribution[LINKED_BUFFER_SIZE + 1] = {0};
#endif
};

#endif  // LINKEDLISTALLOCATOR_H