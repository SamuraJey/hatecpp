#include "PoolAllocator.hh"

#include <cstdlib>
#include <new>

#include "resources/constants.hh"

struct PoolAllocator::Buffer {
    Buffer* prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

PoolAllocator::PoolAllocator() {
    buffer_list_head = nullptr;
    createNewBuffer(POOL_BUFFER_SIZE);
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
    // Placment new - конструкция для создания объекта в уже выделенной памяти
    new (New) Buffer();  // placement new
    New->prev = buffer_list_head;
    New->size = size + sizeof(Buffer);
    buffer_list_head = New;
}

char* PoolAllocator::allocate(size_t size) {
    if (buffer_list_head->current + size > buffer_list_head->size) {
        if (POOL_BUFFER_SIZE >= size) {
            createNewBuffer(POOL_BUFFER_SIZE);
        } else {
            createNewBuffer(size);
        }
        // Тернарный оператор - ещё один вариант
        // createNewBuffer((size > POOL_BUFFER_SIZE) ? (size) : (POOL_BUFFER_SIZE));
    }

    char* ret = reinterpret_cast<char*>(buffer_list_head) + buffer_list_head->current;
    buffer_list_head->current += size;
    return ret;
}
void PoolAllocator::deallocate(void* ptr) {
}
