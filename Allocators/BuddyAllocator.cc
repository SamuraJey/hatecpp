

#include "BuddyAllocator.hh"

#include <algorithm>
#include <iostream>

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
    asm("bsr %1, %0" : "=r"(req_level) : "r"(allocateSize - 1 << 1));
    // немного побитовой магии и цыкла как ни бывало
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