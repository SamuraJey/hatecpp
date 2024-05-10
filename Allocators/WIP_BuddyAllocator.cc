
#ifndef BUDDYALLOCATOR_H
#define BUDDYALLOCATOR_H

#include <algorithm>
#include <iostream>
#include "Allocator.hh"

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 10 * 1024 * 1024
#define DEBUG true
using namespace std;

struct BuddyBlock {
    unsigned char level;
    bool free;
    union {
        struct {
            BuddyBlock *prev, *next;
        };
        char data[1];
    };
};

template <class E>
class List {
    E* root = nullptr;

   public:
    void remove(E* cur) {
        if (cur == cur->next) {
            root = nullptr;
            return;
        }
        E* prev = cur->prev;
        E* next = cur->next;
        prev->next = next; // segfault xD
        next->prev = prev;
        if (root == cur) {
            root = next;
        }
    }




    void insert(E* newBlock) {
        if (root == nullptr) {
            root = newBlock;
            root->prev = root;
            root->next = root;
            return;
        }
        E* prevRoot = root->prev;
        prevRoot->next = newBlock;
        newBlock->prev = prevRoot;
        root->prev = newBlock;
        newBlock->next = root;
    }

    E* getRoot() {
        return root;
    }

    bool isEmpty() {
        return root == nullptr;
    }
};

class BuddyAllocator: public Allocator {
    const size_t maxLevel = 25;  // was M before
    List<BuddyBlock>* list = nullptr;
    char* buffer;

   public:
    BuddyAllocator() {
        list = new List<BuddyBlock>[maxLevel + 1];
        buffer = (char*)malloc(1 << maxLevel);
        BuddyBlock* block = (BuddyBlock*)buffer;
        block->level = maxLevel;
        block->free = true;
        list[maxLevel].insert(block);
    }

    ~BuddyAllocator() {
        delete[] list;
    }

    char* allocate(size_t size) {
        printf("start allocate\n");
        size_t allocateSize = max(sizeof(BuddyBlock), size + 2);
        printf("allocate size: %d\n", allocateSize);
        size_t savedIndex = (size_t)-1;
        for (size_t blockIndex = 0; blockIndex <= maxLevel; blockIndex++) {
            size_t currentSize = 1 << blockIndex;
            if (currentSize >= allocateSize && !list[blockIndex].isEmpty()) {
                savedIndex = blockIndex;
                break;
            }
        }
        if (savedIndex == (size_t)-1) {
            throw std::bad_alloc();
        }
        List<BuddyBlock>& currentList = list[savedIndex];
        BuddyBlock* currentItem = currentList.getRoot();
        size_t currentItemSize = 1 << currentItem->level;
        list->remove(currentItem);
        while (((currentItemSize / 2) >= allocateSize)) {
            currentItemSize /= 2;
            BuddyBlock* left = currentItem;
            BuddyBlock* right = (BuddyBlock*)((char*)currentItem + currentItemSize);
            left->level = right->level = currentItem->level - 1;
            left->free = true;
            right->free = true;
            list[right->level].insert(right);
        }
        currentItem->free = false;
        printf("fact size: %d\n", currentItemSize);
        printf("complete\n");
        return currentItem->data;
    }

    void deallocate(void* temp) {
        char* tempBuffer = (char*)temp;
        BuddyBlock* findBlock = (BuddyBlock*)(tempBuffer - sizeof(BuddyBlock));
        findBlock->free = true;
        size_t level = findBlock->level;
        List<BuddyBlock> currentList = list[level];
        size_t blockIndex = ((char*)findBlock - buffer) / (1 << level);
        bool isFound = false;
        while (true) {
            if (blockIndex % 2 == 0) {
                BuddyBlock* right = (BuddyBlock*)((char*)findBlock + (1 << findBlock->level));
                if (right->free && findBlock->level == right->level) {
                    findBlock->level++;
                    isFound = true;
                    currentList.remove(right);
                }
            } else {
                BuddyBlock* left = (BuddyBlock*)((char*)findBlock - (1 << findBlock->level));
                if (left->free && findBlock->level == left->level) {
                    findBlock = left;
                    findBlock->level++;
                    isFound = true;
                    currentList.remove(left);
                }
            }
            if (!isFound) {
                break;
            }
            isFound = false;
        }

        list[findBlock->level].insert(findBlock);
    }

    void check_memory() {
        printf("Lol dude\n");
        // for (size_t i = 0; i <= maxLevel; i++) {
        //     printf("level %d\n", i);
        //     BuddyBlock* currentBlock = list[i].getRoot();
        //     while (currentBlock != nullptr) {
        //         printf("block %d\n", (char*)currentBlock - buffer);
        //         currentBlock = currentBlock->next;
        //     }
        // }
    }
};

#endif  // !BUDDYALLOCATOR_H