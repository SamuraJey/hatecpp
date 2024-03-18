#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024 * 1024 * 8
// using namespace std;

bool isDelim(char& c) {
    switch (c) {
    case ' ':
    case '\n':
    case '.':
    case ',':
    case '!':
    case '-':
    case ';':
    case ':':
    case '?':
    case '"':
    case '\'':
    case '(':
    case ')':
    case '[':
    case ']':
    case '/':
        return true;
    default:
        return false;
    }
}

class Allocator {
   public:
    virtual char* allocate(size_t count) = 0;
    virtual void deallocate(void* p) = 0;
};

class PoolAllocator : public Allocator {
    struct Buffer {
        Buffer* prev = nullptr;
        size_t current = sizeof(Buffer);
        size_t size = 0;
    };

    Buffer* some_buffer = nullptr;

   public:
    void createNewBuffer(size_t size) {
        Buffer* New = static_cast<Buffer*>(malloc(size + sizeof(Buffer)));
        new (New) Buffer();
        New->prev = some_buffer;
        New->size = size + sizeof(Buffer);
        some_buffer = New;
    }

    PoolAllocator() {
        createNewBuffer(BUFFER_SIZE);
    }

    ~PoolAllocator() {
        while (some_buffer != nullptr) {
            Buffer* prev = some_buffer->prev;
            free(some_buffer);
            some_buffer = prev;
        }
    }

    char* allocate(size_t size) override {
        if (some_buffer->size - some_buffer->current < size) {
            createNewBuffer(std::max(BUFFER_SIZE, (int)size));
        }

        char* ret = reinterpret_cast<char*>(some_buffer) + some_buffer->current;
        some_buffer->current = some_buffer->current + size;
        return ret;
    }
    void deallocate(void*) override {
    }
};

class LinkedListAllocator : public Allocator {
    struct BlockHeader {
        size_t size;
        union {
            struct
            {
                BlockHeader *prev, *next;
            };
            char data[1];
        };
    };
    char* buffer = nullptr;
    BlockHeader* root = nullptr;
    size_t bytes_allocated = 0;               // сумма байт всех запросов
    size_t bytes_used = sizeof(BlockHeader);  // сумма байт занятых не мусором

   public:
    LinkedListAllocator() {
        buffer = static_cast<char*>(malloc(LARGE_BUFFER_SIZE));
        // непрямое создание первого блока во весь буффер
        root = reinterpret_cast<BlockHeader*>(buffer);
        root->size = LARGE_BUFFER_SIZE;
        root->prev = root->next = root;
    }

    void remove_from_free(BlockHeader* cur) {
        BlockHeader* prev = cur->prev;
        BlockHeader* next = cur->next;
        prev->next = next;
        next->prev = prev;
        // проверка на единственность в списке
        if (cur == next) {
            root = nullptr;
        }
        // проверка не удаляем ли мы вершину-корень списка
        else if (cur == root) {
            root = next;
        }
    }

    char* allocate(size_t size) override {
        if (root == nullptr) {
            std::cout << "No Free blocks (or linked list access is lost)" << std::endl;
            throw std::bad_alloc();
        }

        BlockHeader* cur = root;
        while (cur->size < size + sizeof(size_t) && cur != root) {
            cur = cur->next;
        }
        if (cur->size < size + sizeof(size_t)) {
            std::cout << "No Block of sufficient size\nBuffer size: " << LARGE_BUFFER_SIZE
                      << "\nallocated bytes total: " << bytes_allocated << "\nbytes in use total: " << bytes_used
                      << std::endl;
            std::cout << "last alloc request/last checked block capacity: " << size << '/' << cur->size - sizeof(size_t)
                      << std::endl;
            throw std::bad_alloc();
        }

        bytes_allocated += size;
        // от блока достаточного размера отрезаем необходимую часть
        if (cur->size >= size + sizeof(BlockHeader) + sizeof(size_t)) {
            // будем отдавать именно отрезанный кусок, чтобы не соверать лишних действий со списком свободных блоков
            BlockHeader* cuted_block = reinterpret_cast<BlockHeader*>((char*)cur + cur->size - size - sizeof(size_t));
            // std::cout << "error log:\nbuffer: " << (int*)buffer << "\nbuffer end: " << (int*)(buffer +
            // LARGE_BUFFER_SIZE); std::cout << "\nroot: " << root << "\ncur:  " << cur << "\ncuted:" << cuted_block;
            // std::cout <<  "\nsize requested: " << size << "\n\n\n";
            cur->size -= size + sizeof(size_t);
            cuted_block->size = size;
            bytes_used += size + sizeof(size_t);
            return cuted_block->data;
        } else {
            remove_from_free(cur);
            bytes_used += (cur->size - sizeof(BlockHeader));
            return cur->data;
        }
    }
    // TODO FIX переписать. Абсолютно не верно
    void deallocate(void* p) override {
        BlockHeader* cur = reinterpret_cast<BlockHeader*>(p);
        bytes_allocated -= cur->size;  // есть ошибка подсчёта
        // std::cout << "deallocate: " << cur->size <<std::endl;
        cur->next = root;
        cur->prev = root->prev;
        root->prev->next = cur;
        root->prev = cur;
    }
};

template <class T>
class CMyAllocator {
   public:
    typedef T value_type;

    Allocator* allocator = nullptr;

    CMyAllocator() : allocator(nullptr) {
    }

    CMyAllocator(Allocator* ByteAllocator) {
        allocator = ByteAllocator;
    }

    template <class U>
    CMyAllocator(const CMyAllocator<U>& V) : allocator(V.allocator) {
    }

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(allocator->allocate(sizeof(T) * count));
    }

    void deallocate(T* p, size_t count) {
        allocator->deallocate(p);
    }
};

bool cmp(std::pair<const char*, size_t> First, std::pair<const char*, size_t> Second) {
    return First.second > Second.second;
}

class CStringComparator {
   public:
    /*
        A < B --> true
    */
    bool operator()(const char* A, const char* B) const {
        while (true) {
            if (A[0] == B[0]) {
                // A = B
                if (!A[0])
                    return false;

                A++;
                B++;
            } else {
                return A[0] < B[0];
            }
        }
    }
};

char* ReadFromFile(const char* file_name) {
    FILE* file = fopen(file_name, "rb");
    if (file == nullptr) {
        printf("fopen\n");
        printf("Terminal failure: unable to open file \"%s\" for read.\n", file_name);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    long long file_size = ftell(file);
    rewind(file);

    char* ReadBuffer = static_cast<char*>(malloc(file_size + 1));
    printf("file_size = %lld\n", file_size);
    fread(ReadBuffer, 1, file_size, file);
    ReadBuffer[file_size] = '\0';
    fclose(file);

    return ReadBuffer;
}

void TextMapTest(Allocator* allocator, char* TextBuffer) {
    CMyAllocator<char*> WrapperAllocator(allocator);
    // std::map<const char*, unsigned long long, CStringComparator, CMyAllocator<char*>> Map(WrapperAllocator);
    std::map<const char*, size_t, CStringComparator, CMyAllocator<std::pair<const char* const, size_t>>> Map(WrapperAllocator);

    char* Word = strtok(TextBuffer, " \n\t\r");
    while (Word != nullptr) {
        Map[Word]++;
        Word = strtok(nullptr, " \n\t\r");
    }

    std::vector<std::pair<const char*, int>> SortedWords(Map.begin(), Map.end());
    std::sort(SortedWords.begin(), SortedWords.end(), cmp);
    int i = 0;
    int num_of_word = 0;

    for (auto Pair : SortedWords) {
        if (i++ < 10)
            printf("%s: %d\n", Pair.first, Pair.second);
        num_of_word += Pair.second;
    }
    printf("Total number of words: %d\n", num_of_word);

    free(TextBuffer);
}

int main() {
    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    PoolAllocator* poolAllocator = new PoolAllocator();
    char* ReadBuffer = ReadFromFile("war_en.txt");

    auto start1 = std::chrono::high_resolution_clock::now();
    TextMapTest(linkedListAllocator, strdup(ReadBuffer));
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = end1 - start1;
    printf("LinkedListAllocator execution time: %f seconds\n\n", duration1.count());

    auto start2 = std::chrono::high_resolution_clock::now();
    TextMapTest(poolAllocator, strdup(ReadBuffer));
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = end2 - start2;
    printf("PoolAllocator execution time: %f seconds\n\n", duration2.count());

    free(ReadBuffer);
    delete linkedListAllocator;
    delete poolAllocator;
    return 0;
}