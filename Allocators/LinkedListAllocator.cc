#include "LinkedListAllocator.hh"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <new>

#include "../constants.hh"

struct LinkedListAllocator::BlockHeader {
    size_t size;
    union {
        struct
        {
            BlockHeader *prev, *next;
        };
        char data[1];
    };
};

constexpr size_t LinkedListAllocator::F_header = sizeof(LinkedListAllocator::BlockHeader);
constexpr size_t LinkedListAllocator::A_header = sizeof(size_t);

LinkedListAllocator::LinkedListAllocator()
    // Синтаксис для инициализации константных полей. см. Constructors and member initializer lists
    : buffer(static_cast<char*>(malloc(LINKED_BUFFER_SIZE))) {
    root->prev = root->next = root = reinterpret_cast<BlockHeader*>(buffer);
    root->size = LINKED_BUFFER_SIZE;

#if DEBUG
    bytes_allocated = 0;
    max_bytes_used = bytes_used = F_header;
    max_block_count = block_counter = 1;
    block_size_distribution[LINKED_BUFFER_SIZE] = 1;

    printf(
        "Linked List Allocator construction succesful\n\
root: %p, buffer size: %lu\n\n",
        root, LINKED_BUFFER_SIZE);
#endif
}

LinkedListAllocator::~LinkedListAllocator() {
#if DEBUG
    printf(
        "Linked Aist Allocator destructor log\n\
root: addr: %p, size: %lu\n\
bytes left allocated: %lu, bytes left used: %lu, max bytes used: %lu\n\
single block left? %d, block count: %lu, max block count: %lu\n\
block distribution:\n",
        root, root->size,
        bytes_allocated, bytes_used, max_bytes_used,
        root->prev == root && root == root->next, block_counter, max_block_count);

    for (size_t size = 0; size <= LINKED_BUFFER_SIZE; ++size) {
        size_t count = block_size_distribution[size];
        if (count) {
            printf("size %lu blocks %lu\n", size, count);
        }
    }
#endif
    free(buffer);
}

void LinkedListAllocator::mark_used(BlockHeader* rem) {
    BlockHeader* prev = rem->prev;
    BlockHeader* next = rem->next;
    if (rem == root && rem == (root = next)) {
        // Список из 1 удаляемого элемента - забываем
        root = nullptr;
        return;
    }
    prev->next = next;
    next->prev = prev;
    return;
}

char* LinkedListAllocator::allocate(size_t size) {
    if (root == nullptr) {
        printf("No Free blocks (or linked list access is lost)\n");
        throw std::bad_alloc();
    }

    BlockHeader* cur = root;
    do {
        cur = cur->next;  // root - последний элемент итерации
    } while (cur != root && cur->size < size + A_header);

    if (cur->size < size + A_header) {
        printf(
            "No Block of sufficient size error\nBuffer size: %lu\n\
last alloc request/last checked block capacity: %lu/%lu\n",
            LINKED_BUFFER_SIZE, size, cur->size - A_header);
        throw std::bad_alloc();
    }

    // От блока достаточного размера отрезаем новый блок требуемого размера
    if (cur->size >= (size + A_header) + F_header) {
        BlockHeader* cuted_block = reinterpret_cast<BlockHeader*>(((char*)cur + cur->size) - (size + A_header));

#if DEBUG
        bytes_allocated += size;
        bytes_used += size + A_header;
        ++block_counter;
        max_block_count = (block_counter > max_block_count) ? (block_counter) : (max_block_count);
        --block_size_distribution[cur->size];
        ++block_size_distribution[(size + A_header)];
        ++block_size_distribution[cur->size - (size + A_header)];
#endif

        cur->size -= (cuted_block->size = size + A_header);
        return cuted_block->data;
    } else {
        mark_used(cur);

#if DEBUG
        bytes_allocated += cur->size - A_header;
        bytes_used += cur->size - F_header;
        max_bytes_used = (bytes_used > max_bytes_used) ? (bytes_used) : (max_bytes_used);
#endif

        return cur->data;
    }
}

void LinkedListAllocator::deallocate(void* ptr) {
    //!!! p - адрес на начала данных, не блока
    BlockHeader* to_free = reinterpret_cast<BlockHeader*>((char*)ptr - A_header);
#if DEBUG
    bytes_allocated -= to_free->size - A_header;
    bytes_used -= to_free->size - F_header;
    max_bytes_used = (bytes_used > max_bytes_used) ? (bytes_used) : (max_bytes_used);
#endif
    if (!root) {
        root = to_free->prev = to_free->next = to_free;
        return;
    }

    BlockHeader *cur = root, *prev_free = nullptr, *next_free = nullptr;
    do {
        // Запоминаем граничащие свободные блоки
        if ((char*)cur + cur->size == (char*)to_free) {
            prev_free = cur;
        }
        if ((char*)to_free + to_free->size == (char*)cur) {
            next_free = cur;
        }
    } while ((cur = cur->next) != root);  // Самый затратный цыкл

    // Далее обработка 4 случаев. (наличие/отсутствие)*(правого/левого) соседа для слияния
    switch ((bool)prev_free << 1 | (bool)next_free) {
    // Случай 0: соседей нет => вставляем блок за корнем
    case 0:
        (to_free->prev = root)->next = (to_free->next = root->next)->prev = to_free;
        break;
    // Случай 1: только сосед справа => подменяем в списке соседа на себя и присоединяем его
    case 1:
        (to_free->prev = next_free->prev)->next = (to_free->next = next_free->next)->prev = to_free;

#if DEBUG
        bytes_used -= F_header;
        --block_counter;
        --block_size_distribution[to_free->size];
        --block_size_distribution[next_free->size];
        ++block_size_distribution[to_free->size + next_free->size];
#endif
        to_free->size += next_free->size;
        break;
    // Случай 2: только сосед слева => присоеденяемся к нему
    case 2:
#if DEBUG
        bytes_used -= F_header;
        --block_counter;
        --block_size_distribution[prev_free->size];
        --block_size_distribution[to_free->size];
        ++block_size_distribution[prev_free->size + to_free->size];
#endif

        prev_free->size += to_free->size;
        break;
    // Случай 3: оба соседа слева и справа => вырезаем правого и присоединяем себя и правого к левому
    case 3:
        mark_used(next_free);
#if DEBUG
        bytes_used -= 2 * F_header;
        block_counter -= 2;
        --block_size_distribution[prev_free->size];
        --block_size_distribution[to_free->size];
        --block_size_distribution[next_free->size];
        ++block_size_distribution[prev_free->size + to_free->size + next_free->size];
#endif
        prev_free->size += to_free->size + next_free->size;
        break;
    }
}