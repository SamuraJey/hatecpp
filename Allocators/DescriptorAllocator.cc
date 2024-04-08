
#include "DescriptorAllocator.hh"

#include <new>
#include <stdexcept>

#include "../constants.hh"

// Идея 1 - обьединение дескриптора правого блока с хедером левого
// Они итак всегда находились вплотную к друг другу. Это известно ещё в compile time и мы пользуемся этим фактом для расчёта адресов.
// Так почему, тогда, не форализовать этот факт обьедив структуры в одну, последовательную и непрерывную.
// Определение такой структуры  даст компилятору инормацию о взаимном расположении полей в compile time.
// + можно переложить добрую часть арифметики указателей на компилятор в виде обращений к полям хедера
// + компилятор имеет больше возможностей для оптимизаций
// + операции получения концевика и правого соседа вырождаются в одну

// **(за исключением одной проверки в allocate)
// Идея 2 - использование указателей вместо размеров
// Размеры блока (**) используются для перехода к хедерам соседей при обьединении свободных блоков.
// В концевике замена: размер блока => указатель на заголовок
// + меншье расчётов
// + удобный доступ к полям левого соседа через -> нотайию
//
// В заголовке: размер блока => указатель на правого соседа
// + меньше расчётов адресов
// + удобный доступ к полям правого соседа
// - размер нужно вычислять

// Идея 3 - помечать занятость блока не флагом, а состояним размеров/указателей
// Концевик ипользуется только для доступа к заголовку блока справа.
// Заголовок соседа нужен только при склеивании пустых блоков => занятый блок можно пометить концевиком равным нулю (невозможным адресом/размером)
// невозможность получить заголовок занятого левого соседа - не проблема.
// + экономия места (каждый bool из-за выравнивания резервирует 8 байт)
// ? получение состояние левого соеда всё так же не требется разименований
// - получение состояние правого соседа требует 2 (на 1 больше) разименования
// p.s. указатель в заголовке используется для вывидения размера блока.
// При сохранении размера явно этот трюк можно провернуть и с правм соседом (2->0 разиминований).

// Идея 4 - убрать специальную обрабтку крайних блоков буффера в dealocate.
// крайних блоков не будет, если добавив guard-заглушки по краям буфера.

struct DescriptorAllocator::BlockHeader {
    BlockHeader* left_neighbour;
    BlockHeader* right_neighbour;
    union {
        struct {
            BlockHeader *prev, *next;
        };
        char data[1];
    };

    // Вобщем тут много коротких функций, вызываемых по 1 разу.
    // Их можно вписать в вызывающий код и убарть лишние сущности
    // или оставить в виде отдельных вызывов, для формализации кода
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
    : buffer((char*)malloc(DESC_BUFFER_SIZE)),
      endBuffer(buffer + DESC_BUFFER_SIZE) {
    root = (BlockHeader*)(buffer);
    root->left_neighbour = nullptr;
    root->prev = root->next = root;

    // guard region - обрубок заголовка (24 байта с 3 указателями)
    *(char**)(endBuffer - sizeof(void*)) = nullptr;
    *(char**)(endBuffer - 2 * sizeof(void*)) = endBuffer - sizeof(void*);
    root->set_size(DESC_BUFFER_SIZE - 3 * sizeof(void*))->left_neighbour = root;
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
