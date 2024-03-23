#pragma once
#include "Allocator.hh"
#include "BlockHeader.hh"
/*
Я не понял как нормально объявить структуру
что бы она и в хедере была и в реализации,
пока вынес в другой файл, возможно надо сделать по-другому
*/
class LinkedListAllocator : public Allocator {
   public:
    LinkedListAllocator();
    ~LinkedListAllocator();
    void remove_from_list(BlockHeader* rem);
    char* allocate(size_t size) override;
    void deallocate(void* ptr) override;
};