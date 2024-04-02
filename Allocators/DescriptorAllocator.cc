#include <malloc.h>
#include <stdio.h>

#include <new>
#include <stdexcept>

#include "../constants.hh"
#include "Allocator.hh"


using namespace std;


struct Descriptor {
    size_t size;
    bool free;
};

struct BlockHeader_desc {
    Descriptor descriptor;
    union {
        struct {
            BlockHeader_desc *prev, *next;
        };
        char data[1];
    };

    Descriptor* getRightDescriptor() {
        char* rightDescriptor = ((char*)this) + descriptor.size - sizeof(Descriptor);
        return (Descriptor*)rightDescriptor;
    }
};

class DescriptorAllocator : public Allocator{
    char* buffer = nullptr;
    BlockHeader_desc* root = nullptr;
    char* endBuffer = nullptr;

   public:
    DescriptorAllocator() {
        buffer = (char*)malloc(LINKED_BUFFER_SIZE);
        root = (BlockHeader_desc*)buffer;
        endBuffer = buffer + LINKED_BUFFER_SIZE;
        root->descriptor.size = LINKED_BUFFER_SIZE;
        root->descriptor.free = 1;
        root->getRightDescriptor()->free = 1;
        root->getRightDescriptor()->size = LINKED_BUFFER_SIZE;
        root->prev = root->next = root;
    }

    void remove(BlockHeader_desc* cur) {
        cur->descriptor.free = 0;
        cur->getRightDescriptor()->free = 0;
        if (cur == cur->next) {
            root = nullptr;
            return;
        }
        BlockHeader_desc* prev = cur->prev;
        BlockHeader_desc* next = cur->next;
        prev->next = next;
        next->prev = prev;
        if (root == cur) {
            root = next;
        }
    }

    void insert(BlockHeader_desc* newBlock) {
        newBlock->descriptor.free = 1;
        newBlock->getRightDescriptor()->free = 1;
        if (root == nullptr) {
            root = newBlock;
            root->prev = root;
            root->next = root;
            return;
        }
        BlockHeader_desc* prevRoot = root->prev;
        prevRoot->next = newBlock;
        newBlock->prev = prevRoot;
        root->prev = newBlock;
        newBlock->next = root;
    }

    char* allocate(size_t size) {
        if (root == nullptr) {
            // throw bad_alloc();
        }
        BlockHeader_desc* cur = root;
        BlockHeader_desc* result = nullptr;
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
            throw bad_alloc();
        }
        remove(result);
        size_t freeSpace = result->descriptor.size - generalSize;
        if (freeSpace <= sizeof(BlockHeader_desc)) {
            return &result->data[0];
        }
        char* newBuffer = ((char*)result) + generalSize;
        BlockHeader_desc* newBlock = (BlockHeader_desc*)newBuffer;
        newBlock->descriptor.size = freeSpace;
        newBlock->getRightDescriptor()->size = freeSpace;
        insert(newBlock);
        result->descriptor.size = generalSize;
        result->getRightDescriptor()->size = generalSize;
        result->descriptor.free = 0;
        result->getRightDescriptor()->free = 0;
        return &result->data[0];
    }

    void deallocate(void* temp) {
        char* buffer = (char*)temp;
        BlockHeader_desc* findBlock = (BlockHeader_desc*)(buffer - sizeof(Descriptor));
        BlockHeader_desc* left = nullptr;
        BlockHeader_desc* right = nullptr;
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
                left = (BlockHeader_desc*)(((char*)leftDescriptor) - leftDescriptor->size + sizeof(Descriptor));
            }

        if (rightDescriptor)
            if (rightDescriptor->free == 1) {
                right = (BlockHeader_desc*)((char*)rightDescriptor);
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

    void check_memory() {
        BlockHeader_desc* cur = root;
        int i = 0;
        if (root == nullptr)
            throw std::runtime_error("Error: root was nullptr");
        do {
            printf("Buffer_desc #%d, size %d\n", i++, (int)cur->descriptor.size);
            cur = cur->next;
        } while (cur != root);
    }
};