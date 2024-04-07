
#include "DescriptorAllocator.hh"

#include <new>
#include <stdexcept>

#include "../constants.hh"

struct DescriptorAllocator::BlockHeader {
    //
    BlockHeader* left_neighbour;
    // bool free;
    BlockHeader* right_neighbour;
    union {
        struct {
            BlockHeader *prev, *next;
        };
        char data[1];
    };

    inline size_t get_size() const noexcept {
        return (size_t)(right_neighbour) - (size_t)(this);
    }

    inline BlockHeader* set_size(size_t size) noexcept {
        return right_neighbour = (BlockHeader*)((size_t)(this) + size);
    }

    inline bool isFree() const noexcept {
        return right_neighbour->left_neighbour;
    }

    inline void set_free() noexcept {
        right_neighbour->left_neighbour = this;
    }

    inline void set_used() const noexcept {
        right_neighbour->left_neighbour = nullptr;
    }
};

constexpr size_t DescriptorAllocator::free_header = sizeof(BlockHeader);
constexpr size_t DescriptorAllocator::alloc_header = 2 * sizeof(void*);

DescriptorAllocator::DescriptorAllocator()
    : buffer((char*)malloc(LINKED_BUFFER_SIZE)),
      endBuffer(buffer + LINKED_BUFFER_SIZE) {
    // guard region
    // последние 8 байт - nulptr
    // предпоследние 8 байт - указатель на последние 8 байт
    *(char**)(endBuffer - sizeof(void*)) = nullptr;
    *(char**)(endBuffer - 2 * sizeof(void*)) = endBuffer - sizeof(void*);

    root = (BlockHeader*)(buffer);
    root->left_neighbour = nullptr;
    (root->right_neighbour = (BlockHeader*)(endBuffer - 3 * sizeof(void*)))->left_neighbour = root;
    root->prev = root->next = root;
}

DescriptorAllocator::~DescriptorAllocator() {
    check_memory();
    free(buffer);
}

inline void DescriptorAllocator::add_free(BlockHeader* block) {
    if (root == nullptr) {
        root = block->prev = block->next = block;
        return;
    }
    (block->next = root->next)->prev = block;
    (block->prev = root)->next = block;
    return;
}

inline void DescriptorAllocator::pop_free(BlockHeader* block) {
    block->set_used();
    BlockHeader* prev = block->prev;
    BlockHeader* next = block->next;
    if (block == root && block == (root = next)) {
        // Если удоляемый элемент в списке единственный, вырезать его не получится.
        // Затираем ссылку на такой список.
        root = nullptr;
        return;
    }
    prev->next = next;
    next->prev = prev;
    return;
}

char* DescriptorAllocator::allocate(size_t size) {
    if (root == nullptr) {
        throw std::bad_alloc();
    }
    size_t min_req_size = size + alloc_header;

    BlockHeader* cur = root;
    while (cur->get_size() < min_req_size && (cur = cur->next) != root) {
    }
    if (cur->get_size() < min_req_size) {
        throw std::bad_alloc();
    }

    size_t freeSpace = cur->get_size() - min_req_size;
    if (freeSpace < free_header) {
        pop_free(cur);
        return cur->data;
    }

    // Если места достаточно отпиливаем от свободного блока запрошенный кусочек
    BlockHeader* newBlock = cur->set_size(freeSpace);
    newBlock->left_neighbour = cur;
    newBlock->set_size(min_req_size)->left_neighbour = nullptr;
    return newBlock->data;
}

void DescriptorAllocator::deallocate(void* data) {
    BlockHeader* mid_block = (BlockHeader*)((char*)data - alloc_header);
    BlockHeader* left_block = mid_block->left_neighbour;
    BlockHeader* right_block = mid_block->right_neighbour;

    switch ((bool)left_block << 1 | right_block->isFree()) {
    // Случай 0: соседей нет => помечаем блок свободным
    case 0:
        mid_block->set_free();
        add_free(mid_block);
        break;

    // Случай 1: только сосед справа => подменяем в списке соседа на себя и присоединяем его
    case 1:
        (mid_block->prev = right_block->prev)->next = mid_block;
        (mid_block->next = right_block->next)->prev = mid_block;

        (mid_block->right_neighbour = right_block->right_neighbour)->left_neighbour = mid_block;
        break;

    // Случай 2: только сосед слева => присоеденяемся к нему
    case 2:
        (left_block->right_neighbour = right_block)->left_neighbour = left_block;
        break;

    // Случай 3: оба соседа слева и справа => вырезаем правого и присоединяем себя и правого к левому
    case 3:
        BlockHeader* next = right_block->next;
        BlockHeader* prev = right_block->prev;
        prev->next = next;
        next->prev = prev;

        (left_block->right_neighbour = right_block->right_neighbour)->left_neighbour = left_block;
        break;
    }
}

void DescriptorAllocator::check_memory() {
    BlockHeader* cur = root;
    int i = 0;
    if (root == nullptr)
        throw std::runtime_error("Error: root was nullptr");
    do {
        printf(" #%d, Block ptr:%p size %d\n", i++, cur, (int)cur->get_size());
        cur = cur->next;
    } while (cur != root && i < 25);
}
