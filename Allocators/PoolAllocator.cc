#include "PoolAllocator.hh"

#include <cstdlib>
#include <new>

#include "resources/constants.hh"

struct PoolAllocator::Buffer {
    Buffer* prev = nullptr;
};

PoolAllocator::PoolAllocator() {
    buffer_list_head = nullptr;
    space = top = 0;
}

PoolAllocator::~PoolAllocator() {
    while (buffer_list_head != nullptr) {
        Buffer* prev = buffer_list_head->prev;
        free(buffer_list_head);
        buffer_list_head = prev;
    }
}

void PoolAllocator::createNewBuffer(size_t size) {
    Buffer* new_buffer = static_cast<Buffer*>(malloc(size + sizeof(Buffer)));
    space = size + sizeof(Buffer);
    top = sizeof(Buffer);
    new_buffer->prev = buffer_list_head;
    buffer_list_head = new_buffer;
}

char* PoolAllocator::allocate(size_t size) {
    if (top + size > space) createNewBuffer((POOL_BUFFER_SIZE > size) ? (POOL_BUFFER_SIZE) : (size));
    char* ret = reinterpret_cast<char*>(buffer_list_head) + top;
    top += size;
    return ret;
}
void PoolAllocator::deallocate(void* ptr) {
    return;  // that is fast code
}
