#include "PoolAllocator.hh"

#include <malloc.h>
#include <stdlib.h>

#include <new>

#include "../constants.hh"

struct Buffer {
    Buffer* prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

Buffer* some_buffer = nullptr;

PoolAllocator::PoolAllocator() {
    createNewBuffer(BUFFER_SIZE);
}

PoolAllocator::~PoolAllocator() {
    while (some_buffer != nullptr) {
        Buffer* prev = some_buffer->prev;
        free(some_buffer);
        some_buffer = prev;
    }
}

void PoolAllocator::createNewBuffer(size_t size) {
    Buffer* New = static_cast<Buffer*>(malloc(size + sizeof(Buffer)));
    new (New) Buffer();
    New->prev = some_buffer;
    New->size = size + sizeof(Buffer);
    some_buffer = New;
}

char* PoolAllocator::allocate(size_t size) {
    if (some_buffer->size - some_buffer->current < size) {
        if (BUFFER_SIZE >= (int)size) {
            createNewBuffer(BUFFER_SIZE);
        } else {
            createNewBuffer((int)size);
        }
        // Была ошибка что std не содержит max... Проще ифом сделать, ифы работают всегда
        // createNewBuffer(std::max(BUFFER_SIZE, (int)size));
    }

    char* ret = reinterpret_cast<char*>(some_buffer) + some_buffer->current;
    some_buffer->current = some_buffer->current + size;
    return ret;
}
void PoolAllocator::deallocate(void*) {
}
