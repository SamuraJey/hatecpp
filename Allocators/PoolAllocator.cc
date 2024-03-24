#include "PoolAllocator.hh"

#include <malloc.h>
#include <stdlib.h>

#include <new>

#include "../constants.hh"

struct PoolAllocator::Buffer {
    Buffer* prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

// буффер должен принадлежать экземпляру аллокатора
// несколько pool аллокаторов будут работать с одной переменной
// и как some_buffer работал без метки static - для меня загадка

PoolAllocator::PoolAllocator() {
    createNewBuffer(BUFFER_SIZE);
}

PoolAllocator::~PoolAllocator() {
    while (buffer_list_head != nullptr) {
        Buffer* prev = buffer_list_head->prev;
        free(buffer_list_head);
        buffer_list_head = prev;
    }
}

void PoolAllocator::createNewBuffer(size_t size) {
    Buffer* New = static_cast<Buffer*>(malloc(size + sizeof(Buffer)));
    new (New) Buffer();
    New->prev = buffer_list_head;
    New->size = size + sizeof(Buffer);
    buffer_list_head = New;
}

char* PoolAllocator::allocate(std::size_t size) {
    if (buffer_list_head->size - buffer_list_head->current < size) {
        if (BUFFER_SIZE >= (int)size) {
            createNewBuffer(BUFFER_SIZE);
        } else {
            createNewBuffer((int)size);
        }
        // std::max Defined in header <algorithm>
        // тернарный оператор - ещё один вариант
        // createNewBuffer((size > BUFFER_SIZE) ? (size) : (Buffer));
    }

    char* ret = reinterpret_cast<char*>(buffer_list_head) + buffer_list_head->current;
    buffer_list_head->current = buffer_list_head->current + size;
    return ret;
}
void PoolAllocator::deallocate(void*) {
}
