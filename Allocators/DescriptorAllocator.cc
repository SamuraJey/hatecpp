
#include "DescriptorAllocator.hh"

#include <new>
#include <stdexcept>

#include "../constants.hh"

struct DescriptorAllocator::Descriptor {
    size_t size;
    bool free;
};

struct DescriptorAllocator::BlockHeader {
    Descriptor descriptor;
    union {
        struct {
            BlockHeader *prev, *next;
        };
        char data[1];
    };

    Descriptor* getRightDescriptor() {
        char* rightDescriptor = ((char*)this) + descriptor.size - sizeof(Descriptor);
        return (Descriptor*)rightDescriptor;
    }
};

DescriptorAllocator::DescriptorAllocator() {
    buffer = (char*)malloc(LINKED_BUFFER_SIZE);
    root = (BlockHeader*)buffer;
    endBuffer = buffer + LINKED_BUFFER_SIZE;
    root->descriptor.size = LINKED_BUFFER_SIZE;
    root->descriptor.free = 1;
    root->getRightDescriptor()->free = 1;
    root->getRightDescriptor()->size = LINKED_BUFFER_SIZE;
    root->prev = root->next = root;
}

void DescriptorAllocator::remove(BlockHeader* cur) {
    cur->descriptor.free = 0;
    cur->getRightDescriptor()->free = 0;
    if (cur == cur->next) {
        root = nullptr;
        return;
    }
    BlockHeader* prev = cur->prev;
    BlockHeader* next = cur->next;
    prev->next = next;
    next->prev = prev;
    if (root == cur) {
        root = next;
    }
}

void DescriptorAllocator::insert(BlockHeader* newBlock) {
    newBlock->descriptor.free = 1;
    newBlock->getRightDescriptor()->free = 1;
    if (root == nullptr) {
        root = newBlock;
        root->prev = root;
        root->next = root;
        return;
    }
    BlockHeader* prevRoot = root->prev;
    prevRoot->next = newBlock;
    newBlock->prev = prevRoot;
    root->prev = newBlock;
    newBlock->next = root;
}

char* DescriptorAllocator::allocate(size_t size) {
    if (root == nullptr) {
        throw std::bad_alloc();
    }
    BlockHeader* cur = root;
    BlockHeader* result = nullptr;
    size_t generalSize = size + sizeof(Descriptor) * 2;
    while (true) {
        if (cur->descriptor.size >= generalSize) {
            result = cur;
            break;
        }
        cur = cur->next;
        if (cur == root) {
            break;
        }
    }
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    remove(result);
    size_t freeSpace = result->descriptor.size - generalSize;
    if (freeSpace <= sizeof(BlockHeader)) {
        return &result->data[0];
    }
    char* newBuffer = ((char*)result) + generalSize;
    BlockHeader* newBlock = (BlockHeader*)newBuffer;
    newBlock->descriptor.size = freeSpace;
    newBlock->getRightDescriptor()->size = freeSpace;
    insert(newBlock);
    result->descriptor.size = generalSize;
    result->getRightDescriptor()->size = generalSize;
    result->descriptor.free = 0;
    result->getRightDescriptor()->free = 0;
    return &result->data[0];
}

void DescriptorAllocator::deallocate(void* temp) {
    char* buffer = (char*)temp;
    BlockHeader* findBlock = (BlockHeader*)(buffer - sizeof(Descriptor));
    BlockHeader* left = nullptr;
    BlockHeader* right = nullptr;
    Descriptor* leftDescriptor = (Descriptor*)((char*)findBlock - sizeof(Descriptor));
    Descriptor* rightDescriptor = (Descriptor*)((char*)findBlock + findBlock->descriptor.size);

    if ((char*)findBlock == buffer) {
        leftDescriptor = nullptr;
    }

    if ((char*)findBlock + findBlock->descriptor.size == endBuffer) {
        rightDescriptor = nullptr;
    }

    if (leftDescriptor)
        if (leftDescriptor->free == 1) {
            left = (BlockHeader*)(((char*)leftDescriptor) - leftDescriptor->size + sizeof(Descriptor));
        }

    if (rightDescriptor)
        if (rightDescriptor->free == 1) {
            right = (BlockHeader*)((char*)rightDescriptor);
        }

    if (right != nullptr) {
        remove(right);
        findBlock->descriptor.size += right->descriptor.size;
        findBlock->getRightDescriptor()->size = findBlock->descriptor.size;
    }
    insert(findBlock);
    if (left != nullptr) {
        remove(findBlock);
        left->descriptor.size += findBlock->descriptor.size;
        left->getRightDescriptor()->size = left->descriptor.size;
        left->descriptor.free = 1;
        left->getRightDescriptor()->free = 1;
    }
}

void DescriptorAllocator::check_memory() {
    BlockHeader* cur = root;
    int i = 0;
    if (root == nullptr)
        throw std::runtime_error("Error: root was nullptr");
    do {
        printf("Buffer_desc #%d, size %d\n", i++, (int)cur->descriptor.size);
        cur = cur->next;
    } while (cur != root);
}
