

#include "BuddyAllocator.hh"

#include <algorithm>
#include <cstdint>
#include <iostream>

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
    : buffer((char*)malloc(1 << BADDY_MAX_LEVEL)) {
    BlockHeader* block = reinterpret_cast<BlockHeader*>(buffer);
    block->level = BADDY_MAX_LEVEL;
    block->free = true;
    levelLists[BADDY_MAX_LEVEL].put(block);
}

BuddyAllocator::~BuddyAllocator() {
#if DEBUG
    check_memory();
#endif
    delete[] buffer;
}

char* BuddyAllocator::allocate(size_t size) {
    size_t allocateSize = std::max(sizeof(BlockHeader), size + offsetof(BlockHeader, data));
    size_t blockIndex = 0;
    while ((blockIndex <= BADDY_MAX_LEVEL) && (1 << blockIndex < allocateSize || levelLists[blockIndex].isEmpty())) ++blockIndex;
    if (blockIndex > BADDY_MAX_LEVEL) throw std::bad_alloc();

    List<BlockHeader>& currentList = levelLists[blockIndex];
    BlockHeader* currentItem = currentList.pop();
    size_t currentItemSize = 1 << currentItem->level;
    while (((currentItemSize / 2) >= allocateSize)) {
        currentItemSize /= 2;
        BlockHeader* cuted = (BlockHeader*)((char*)currentItem + currentItemSize);
        currentItem->level = cuted->level = currentItem->level - 1;
        cuted->free = true;
        levelLists[cuted->level].put(cuted);
    }
    currentItem->free = false;

    return currentItem->data;
}

void BuddyAllocator::deallocate(void* ptr) {
    BlockHeader* block = reinterpret_cast<BlockHeader*>((char*)ptr - offsetof(BlockHeader, data));

    size_t level = block->level;
    size_t block_idx = Block_idx(block, level);
    BlockHeader* neighbor = Block_ptr(block_idx ^ 1, level);

    while (level < BADDY_MAX_LEVEL && neighbor->level == level && neighbor->free) {
        // вызов remove на currentList, который не обновлялся при итерировании.
        levelLists[level].remove(neighbor);
        block_idx >>= 1;
        ++level;
        neighbor = Block_ptr(block_idx ^ 1, level);
        BlockHeader* neighbor = Block_ptr(block_idx ^ 1, level);
    }
    block = Block_ptr(block_idx, level);
    block->level = level;

    block->free = true;
    levelLists[level].put(block);
}

void BuddyAllocator::check_memory() {
    printf("Lol dude\n");
    for (size_t i = 0; i <= BADDY_MAX_LEVEL; i++) {
        printf("level %d, size %d\n", i, 1 << i);
        BlockHeader *root = levelLists[i].getRoot(), *cur = root;
        if (root == nullptr) continue;

        do {
            printf("block idx %d\n", Block_idx(cur, i));
        } while ((cur = cur->next) != root);
    }
}

inline size_t BuddyAllocator::Block_idx(BlockHeader* block_ptr, size_t block_level) noexcept {
    static uintptr_t bufferOfset = reinterpret_cast<uintptr_t>(buffer);
    return (reinterpret_cast<uintptr_t>(block_ptr) - bufferOfset) / (1 << block_level);
}

inline BuddyAllocator::BlockHeader* BuddyAllocator::Block_ptr(size_t block_idx, size_t block_level) noexcept {
    return reinterpret_cast<BlockHeader*>(buffer + (1 << block_level) * block_idx);
}
