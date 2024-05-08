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
        return reinterpret_cast<Descriptor*>(reinterpret_cast<char*>(this) + descriptor.block_size - sizeof(Descriptor));
    }

    // синхронно меняет размер дескриптолров блока и возвращает ссылку на правый (для других махинаций если нужно)
    DescriptorAllocator::Descriptor* sync_size_set(size_t size) {
        descriptor.block_size = size;
        Descriptor* linked_disk = reinterpret_cast<Descriptor*>(reinterpret_cast<char*>(this) + size - sizeof(Descriptor));
        linked_disk->block_size = size;
        return linked_disk;
    }
};

DescriptorAllocator::DescriptorAllocator()
    : buffer(static_cast<char*>(malloc(LINKED_BUFFER_SIZE))),
      endBuffer(buffer + LINKED_BUFFER_SIZE) {
    // Обрамляющие дескрипторы для корректного объединения крайних блоков штатной логикой
    reinterpret_cast<Descriptor*>(buffer)->free = false;
    (reinterpret_cast<Descriptor*>(endBuffer) - 1)->free = false;

    root = reinterpret_cast<BlockHeader*>(buffer + sizeof(Descriptor));
    root->descriptor.block_size = LINKED_BUFFER_SIZE - 2 * sizeof(Descriptor);
    root->getRightDescriptor()->block_size = LINKED_BUFFER_SIZE - 2 * sizeof(Descriptor);
    root->descriptor.free = true;
    root->getRightDescriptor()->free = true;
    root->prev = root->next = root;
}

DescriptorAllocator::~DescriptorAllocator() {
    check_memory();
    free(buffer);
}

void DescriptorAllocator::mark_used(BlockHeader* block) {
    block->descriptor.free = false;
    block->getRightDescriptor()->free = false;
    BlockHeader* prev = block->prev;
    BlockHeader* next = block->next;
    if (block == root && block == (root = next)) {
        // Если удаляемый элемент в списке единственный, вырезать его не получится.
        // Затираем ссылку на такой список.
        root = nullptr;
        return;
    }
    prev->next = next;
    next->prev = prev;
    return;
}

void DescriptorAllocator::mark_free(BlockHeader* block) {
    block->descriptor.free = true;
    block->getRightDescriptor()->free = true;
    if (root == nullptr) {
        root = block->prev = block->next = block;
        root = block;
        return;
    }
    (block->prev = root->prev)->next = block;
    (block->next = root)->prev = block;
    return;
}

char* DescriptorAllocator::allocate(size_t size) {
    if (root == nullptr) {
        throw std::bad_alloc();
    }
    size_t min_req_size = size + 2 * sizeof(Descriptor);

    BlockHeader* cur = root;
    while (cur->descriptor.block_size < min_req_size && (cur = cur->next) != root) {
    }
    if (cur->descriptor.block_size < min_req_size) {
        throw std::bad_alloc();
    }

    // Не надо вырезать блок и сразу после добавлять другой.
    // Или подменить в списке первый вторым
    // Или вовсе не делать ничего: отдавать отрезанную часть, а первую половину оставить в покое (в списке свободных)
    size_t freeSpace = cur->descriptor.block_size - min_req_size;
    // Пустой блок не меньше sizeof(BlockHeader) + sizeof(Descriptor)
    if (freeSpace < sizeof(BlockHeader) + sizeof(Descriptor)) {
        mark_used(cur);
        return cur->data;
    }

    // Если места достаточно отпиливаем от свободного блока запрошенный кусочек
    cur->descriptor.block_size = freeSpace;
    cur->getRightDescriptor()->block_size = freeSpace;
    cur->getRightDescriptor()->free = true;

    BlockHeader* newBlock = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(cur) + freeSpace);
    newBlock->descriptor.block_size = min_req_size;
    newBlock->getRightDescriptor()->block_size = min_req_size;
    newBlock->descriptor.free = false;
    newBlock->getRightDescriptor()->free = false;
    return newBlock->data;
}

void DescriptorAllocator::deallocate(void* data) {
    BlockHeader* mid_block = reinterpret_cast<BlockHeader*>(static_cast<char*>(data) - sizeof(Descriptor));
    Descriptor* leftDescriptor = reinterpret_cast<Descriptor*>(mid_block) - 1;
    Descriptor* rightDescriptor = reinterpret_cast<Descriptor*>(reinterpret_cast<char*>(mid_block) + mid_block->descriptor.block_size);
    BlockHeader* left_block;
    BlockHeader* right_block;

    switch (leftDescriptor->free << 1 | rightDescriptor->free) {
    // Случай 0: соседей нет => помечаем блок свободным
    case 0:
        mark_free(mid_block);
        break;

    // Случай 1: только сосед справа => подменяем в списке соседа на себя и присоединяем его
    case 1:
        right_block = reinterpret_cast<BlockHeader*>(rightDescriptor);
        (mid_block->prev = right_block->prev)->next = mid_block;
        (mid_block->next = right_block->next)->prev = mid_block;

        mid_block->descriptor.block_size += rightDescriptor->block_size;
        mid_block->getRightDescriptor()->block_size = mid_block->descriptor.block_size;

        mid_block->descriptor.free = true;
        break;

    // Случай 2: только сосед слева => присоеденяемся к нему
    case 2:
        left_block = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(mid_block) - leftDescriptor->block_size);
        left_block->descriptor.block_size += mid_block->descriptor.block_size;
        left_block->getRightDescriptor()->block_size = left_block->descriptor.block_size;
        left_block->getRightDescriptor()->free = true;
        break;

    // Случай 3: оба соседа слева и справа => вырезаем правого и присоединяем себя и правого к левому
    case 3:
        left_block = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(mid_block) - leftDescriptor->block_size);
        right_block = reinterpret_cast<BlockHeader*>(rightDescriptor);

        BlockHeader* next = right_block->next;
        BlockHeader* prev = right_block->prev;
        (prev->next = next)->prev = prev;

        left_block->descriptor.block_size += mid_block->descriptor.block_size + right_block->descriptor.block_size;
        left_block->getRightDescriptor()->block_size = left_block->descriptor.block_size;
        break;
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
