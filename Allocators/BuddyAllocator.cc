

#include "BuddyAllocator.hh"

#include <algorithm>
#include <iostream>

static_assert(sizeof(void*) == 8, "PC is not x86");

DBG(static uint64_t allocate_cnt1 = 0;
    static uint64_t allocate_cnt2 = 0;
    static uint64_t deallocate_cnt1 = 0;
    static uint64_t deallocate_cnt2 = 0;)

struct BuddyAllocator::BlockHeader {
    unsigned char level;
    bool free;
    union {
        struct {
            BlockHeader *prev, *next;
        };
        char data[1];
    };
};

BuddyAllocator::BuddyAllocator()
    : buffer((char*)malloc(1 << BUDDY_MAX_LEVEL)) {
    BlockHeader* block = reinterpret_cast<BlockHeader*>(buffer);
    block->level = BUDDY_MAX_LEVEL;
    block->free = true;
    levelLists[BUDDY_MAX_LEVEL].put(block);
    listsMask = 1 << BUDDY_MAX_LEVEL;
}

BuddyAllocator::~BuddyAllocator() {
    DBG(check_memory(); printf("\nallocate calls: %d\n allocate while: %d\ndealocate calls: %d\ndealocate while: %d\n", allocate_cnt1, allocate_cnt2, deallocate_cnt1, deallocate_cnt2);)
    delete[] buffer;
}

char* BuddyAllocator::allocate(size_t size) {
    DBG(++allocate_cnt1;)
    size_t allocateSize = std::max(sizeof(BlockHeader), size + offsetof(BlockHeader, data));
    size_t req_level;
    /*
    В этом фрагменте кода используется ассемблерная инструкция для эффективного определения наименьшего уровня в иерархии списков, необходимого для выделения запрошенного размера памяти.
    allocateSize <= block_size = 2^req_level
    req_level >= log2(allocateSize)
    Команда "bsr" (Bit Scan Reverse) находит индекс наибольшего значащего бита в аргументе и эквивалентна округлённому вниз log2(x).

    Зачем конструкция `allocateSize - 1 << 1`?
    Это выражение позволяет округлять логарифм вверх.
    `allocateSize - 1` гарантирует, что если размер выделения уже является степенью двойки, мы не переходим на следующий уровень. Например, если `allocateSize` равен 8 (который является 2^3), вычитание 1 даст 7, и индекс наибольшего значащего бита будет 2, что соответствует уровню 3.
    Побитовый сдвиг на 1 (`<< 1`) затем используется для учета того, что уровни начинаются с 0. Это гарантирует, что для `allocateSize`, равного 1 (2^0), мы получим уровень 0, а не -1.
    P.S. С математической точки зрения log2((x-1)*2) = log2(x-1)+1. Первый вариант выбран из-за отличия "bsr" от log2() в нуле.
    log2(1) = bsr(1) = 0
    log2(0) = -inf, bsr(0) = 0
    Форма bsr(allocateSize - 1 << 1) позволяет избежать ошибки при allocateSize = 1.
    */
    asm("bsr %1, %0" : "=r"(req_level) : "r"(allocateSize - 1 << 1));

    /*
    Здесь используется intrinsic, функция-обёртка, предоставляемая компилятором, "ctzll" (count trailing zeros long long).
    Она находит число идущих подряд нулей справа или, другими словами, индекс младшего бита-единицы.
    `listMask >> req_level` отсекает слишком маленькие уровни, а `req_level + ctzll()` находит наименьший подходящий уровень, у которого имеются свободные блоки.
    */
    size_t level = req_level + __builtin_ctzll(listsMask >> req_level);
    if (level > BUDDY_MAX_LEVEL)
        throw std::bad_alloc();
    listsMask -= 1 << req_level;

    BlockHeader* block_ptr = levelLists[level].pop();

    while (level > req_level) {
        DBG(++allocate_cnt2;)
        --level;
        BlockHeader* restored = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(block_ptr) + (1 << level));
        restored->level = level;
        restored->free = true;
        levelLists[level].put(restored);
    }
    block_ptr->level = req_level;
    block_ptr->free = false;

    return block_ptr->data;
}

void BuddyAllocator::deallocate(void* ptr) {
    DBG(++deallocate_cnt1;)
    BlockHeader* block = reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - offsetof(BlockHeader, data));
    size_t level = block->level;
    // Блок и его приятель стоят вплотную. Т.е. их индексы отличаются на 1.
    // Индекс левого блока в паре - чётный, у правого соответственно - нечётный.
    // Двоичные представления индексов этих блоков одинаковы, за исключением отличающегося бита едениц.
    // Поэтому перевернув младший бит индекса получим индекс соседа
    size_t block_idx = Block_idx(block, level);
    BlockHeader* neighbor = Block_ptr(block_idx ^ 1, level);

    if (level < BUDDY_MAX_LEVEL && neighbor->level == level && neighbor->free) {
        do {
            DBG(++deallocate_cnt2;)
            listsMask ^= levelLists[level].remove(neighbor) << level;
            // Обьединение пары даёт блок следующего уровня двойного размера. Поэтому индекс такого блока будет вдвое меньше.
            // Note индекс объединения пары блоков = общей части индексов этих блоков (без последнего отличающегося бита)
            // Note2 индексация аналогична реализации кучи на массиве. Блок и Приятель = предок и брат, их обьединение = родитель
            block_idx >>= 1;
            ++level;
            neighbor = Block_ptr(block_idx ^ 1, level);
        } while (level < BUDDY_MAX_LEVEL && neighbor->level == level && neighbor->free);
        block = Block_ptr(block_idx, level);
        block->level = level;
    }
    block->free = true;
    levelLists[level].put(block);
    listsMask |= 1 << level;
}

void BuddyAllocator::check_memory() {
    uint64_t padding_length = BUDDY_MAX_LEVEL * 3 / 10 + 1;
    printf("level%*.ssize  mask  blocks", padding_length - 5, "");
    for (size_t i = 0; i <= BUDDY_MAX_LEVEL; i++) {
        BlockHeader *root = levelLists[i].getRoot(), *cur = root;
        uint64_t counter = 0;
        if (root != nullptr) {
            do {
                ++counter;
            } while ((cur = cur->next) != root);
        }
        printf("\n%2llu  %*llu     %llu  %6llu", i, padding_length, 1 << i, listsMask >> i & 1, counter);
    }
}

inline size_t BuddyAllocator::Block_idx(BlockHeader* block_ptr, size_t block_level) noexcept {
    static uintptr_t bufferOfset = reinterpret_cast<uintptr_t>(buffer);
    return (reinterpret_cast<uintptr_t>(block_ptr) - bufferOfset) >> block_level;
}

inline BuddyAllocator::BlockHeader* BuddyAllocator::Block_ptr(size_t block_idx, size_t block_level) noexcept {
    return reinterpret_cast<BlockHeader*>(buffer + (block_idx << block_level));
}