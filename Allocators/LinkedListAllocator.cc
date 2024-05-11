#include "LinkedListAllocator.hh"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <new>

#include "resources/constants.hh"

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

constexpr size_t LinkedListAllocator::free_header = sizeof(LinkedListAllocator::BlockHeader);
constexpr size_t LinkedListAllocator::alloc_header = sizeof(size_t);

LinkedListAllocator::LinkedListAllocator()
    // Синтаксис для инициализации константных полей. см. Constructors and member initializer lists
    : buffer(static_cast<char*>(malloc(LINKED_BUFFER_SIZE))) {
    root->prev = root->next = root = reinterpret_cast<BlockHeader*>(buffer);
    root->size = LINKED_BUFFER_SIZE;

#if DEBUG
    bytes_allocated = 0;
    max_bytes_used = bytes_used = free_header;
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
        // Если удоляемый элемент в списке единственный, вырезать его не получится.
        // Затираем ссылку на такой список.
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
    size_t min_req_size = size + alloc_header;

    // итерируемся пока текущий блок слишком мал и ещё остались не проверенные блоки
    BlockHeader* cur = root;
    while (cur->size < min_req_size && (cur = cur->next) != root) {
    }
    if (cur->size < min_req_size) {
        printf(
            "No Block of sufficient size error\nBuffer size: %lu\n\
last alloc request/last checked block capacity: %lu/%lu\n",
            LINKED_BUFFER_SIZE, size, cur->size - alloc_header);
        throw std::bad_alloc();
    }

    // От блока достаточного размера будем отрезать кусок требуемого размера
    size_t spare_space = cur->size - min_req_size;
    if (spare_space < free_header) {
#if DEBUG
        bytes_allocated += cur->size - alloc_header;
        bytes_used += cur->size - free_header;
        max_bytes_used = (bytes_used > max_bytes_used) ? (bytes_used) : (max_bytes_used);
#endif
        mark_used(cur);
        return cur->data;
    }

#if DEBUG
    bytes_allocated += size;
    bytes_used += min_req_size;
    ++block_counter;
    max_block_count = (block_counter > max_block_count) ? (block_counter) : (max_block_count);
    --block_size_distribution[cur->size];
    ++block_size_distribution[min_req_size];
    ++block_size_distribution[spare_space];
#endif
    BlockHeader* cuted_block = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(cur) + spare_space);
    cur->size = spare_space;
    cuted_block->size = min_req_size;
    return cuted_block->data;
}

void LinkedListAllocator::deallocate(void* ptr) {
    //!!! ptr - адрес на начала данных, не блока
    BlockHeader* to_free = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(ptr) - alloc_header);
#if DEBUG
    bytes_allocated -= to_free->size - alloc_header;
    bytes_used -= to_free->size - free_header;
    max_bytes_used = (bytes_used > max_bytes_used) ? (bytes_used) : (max_bytes_used);
#endif
    if (!root) {
        root = to_free->prev = to_free->next = to_free;
        return;
    }

    BlockHeader *cur = root, *prev_free = nullptr, *next_free = nullptr;
    do {
        // Запоминаем граничащие свободные блоки
        if (reinterpret_cast<char*>(cur) + cur->size == reinterpret_cast<char*>(to_free)) {
            prev_free = cur;
        }
        if (reinterpret_cast<char*>(to_free) + to_free->size == reinterpret_cast<char*>(cur)) {
            next_free = cur;
        }
    } while ((cur = cur->next) != root);  // Самый затратный цыкл

    // Далее обработка 4 случаев. (наличие/отсутствие)*(правого/левого) соседа для слияния
    switch (static_cast<bool>(prev_free) << 1 | static_cast<bool>(next_free)) {
    // Случай 0: соседей нет => вставляем блок за корнем
    case 0:
        (to_free->prev = root)->next = (to_free->next = root->next)->prev = to_free;
        break;

    // Случай 1: только сосед справа => подменяем в списке соседа на себя и присоединяем его
    case 1:
        (to_free->prev = next_free->prev)->next = (to_free->next = next_free->next)->prev = to_free;

#if DEBUG
        bytes_used -= free_header;
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
        bytes_used -= free_header;
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
        bytes_used -= 2 * free_header;
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