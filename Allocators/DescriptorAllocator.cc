
#include "DescriptorAllocator.hh"

#include <new>
#include <stdexcept>

#include "../constants.hh"

struct DescriptorAllocator::Descriptor {
    size_t block_size;  // получалась обманчивая нотация BlockHeader.Descriptor.size читался как размер дискриптора, а не всго блока
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
    // эта функция всегда вызывается после работы с левым дескриптором. (искл 101 строка)
    // мб стоит добавить функции для синхронной работы с обоими дескрипторами для сокращения кода
    Descriptor* getRightDescriptor() {
        char* rightDescriptor = ((char*)this) + descriptor.block_size - sizeof(Descriptor);
        return (Descriptor*)rightDescriptor;
    }
};

DescriptorAllocator::DescriptorAllocator()
    : buffer((char*)malloc(LINKED_BUFFER_SIZE)),
      endBuffer(buffer + LINKED_BUFFER_SIZE) {
    root = (BlockHeader*)buffer;
    root->descriptor.block_size = LINKED_BUFFER_SIZE;
    root->getRightDescriptor()->block_size = LINKED_BUFFER_SIZE;
    root->descriptor.free = 1;
    root->getRightDescriptor()->free = 1;
    root->prev = root->next = root;
}

void DescriptorAllocator::mark_used(BlockHeader* cur) {
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

void DescriptorAllocator::mark_unused(BlockHeader* newBlock) {
    newBlock->descriptor.free = 1;
    newBlock->getRightDescriptor()->free = 1;
    if (root == nullptr) {
        newBlock->prev = newBlock->next = newBlock;
        root = newBlock;
        return;
    }
    (newBlock->prev = root->prev)->next = newBlock;
    (newBlock->next = root)->prev = newBlock;
}

char* DescriptorAllocator::allocate(size_t size) {
    if (root == nullptr) {
        throw std::bad_alloc();
    }
    BlockHeader* cur = root;
    size_t searchSize = size + sizeof(Descriptor) * 2;
    while (true) {
        if (cur->descriptor.block_size >= searchSize) {
            break;
        }
        cur = cur->next;
        if (cur == root) {
            break;
        }
    }
    if (cur->descriptor.block_size < searchSize) {
        throw std::bad_alloc();
    }

    // Не надо вырезать блок и сразу после добавлять другой.
    // Или подменить в списке первый вторым
    // или вовсе не делать ничего: отдовать отрезанную часть, а первую половину оставить в покое (в списке свободных)
    size_t freeSpace = cur->descriptor.block_size - searchSize;
    // пустой блок не меньше sizeof(BlockHeader) + sizeof(Descriptor)
    if (freeSpace < sizeof(BlockHeader) + sizeof(Descriptor)) {
        mark_used(cur);
        return cur->data;
    }

    // Если места достаточно отпиливаем от свободного блока запрошенный кусочек
    cur->descriptor.block_size = freeSpace;
    cur->getRightDescriptor()->block_size = freeSpace;
    cur->getRightDescriptor()->free = 1;

    BlockHeader* newBlock = (BlockHeader*)((char*)cur + freeSpace);
    newBlock->descriptor.block_size = searchSize;
    newBlock->getRightDescriptor()->block_size = searchSize;
    newBlock->descriptor.free = 0;
    newBlock->getRightDescriptor()->free = 0;
    return newBlock->data;
}

// Всё классно, но переделывать. Завтра займусь.
void DescriptorAllocator::deallocate(void* data) {
    BlockHeader* dataBlock = (BlockHeader*)((char*)data - sizeof(Descriptor));
    BlockHeader* left = nullptr;
    BlockHeader* right = nullptr;
    Descriptor* leftDescriptor = (Descriptor*)((char*)dataBlock - sizeof(Descriptor));
    Descriptor* rightDescriptor = (Descriptor*)((char*)dataBlock + dataBlock->descriptor.block_size);  // уф, что творят...

    // мб добавить "занятые" дескрипторы-заглушки в начало и конец буфера. Чтобы крайние случаи проверялись штатной логикой.
    if ((char*)dataBlock == buffer) {
        leftDescriptor = nullptr;
    }
    if ((char*)dataBlock + dataBlock->descriptor.block_size == endBuffer) {
        rightDescriptor = nullptr;
    }

    if (leftDescriptor)
        if (leftDescriptor->free) {
            left = (BlockHeader*)((char*)leftDescriptor - (leftDescriptor->block_size - sizeof(Descriptor)));
        }

    if (rightDescriptor)
        if (rightDescriptor->free) {
            right = (BlockHeader*)((char*)rightDescriptor);
        }

    if (right) {
        mark_used(right);
        dataBlock->descriptor.block_size += right->descriptor.block_size;
        dataBlock->getRightDescriptor()->block_size = dataBlock->descriptor.block_size;
    }
    // опять же не надо вырезать из связанного списка один элемент, и после добавлять другой
    mark_unused(dataBlock);
    if (left) {
        mark_used(dataBlock);
        left->descriptor.block_size += dataBlock->descriptor.block_size;
        left->getRightDescriptor()->block_size = left->descriptor.block_size;
        left->descriptor.free = 1;  //?
        left->getRightDescriptor()->free = 1;
    }
}

void DescriptorAllocator::check_memory() {
    BlockHeader* cur = root;
    int i = 0;
    if (root == nullptr)
        throw std::runtime_error("Error: root was nullptr");
    do {
        printf("Buffer_desc #%d, size %d\n", i++, (int)cur->descriptor.block_size);
        cur = cur->next;
    } while (cur != root);
}
