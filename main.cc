#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#include "PoolAllocator.hh"
#include "constants.hh" // Инклуд заголовка с константами типа BUFFER_SIZE и DEBUG
// #define BUFFER_SIZE 1024
// #define LARGE_BUFFER_SIZE 1024 * 1024 * 3
// #define DEBUG 1

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
    // Alias for Free header size
    static constexpr std::size_t F_hrader = sizeof(BlockHeader);
    // Alias for Allocated header size
    static constexpr std::size_t A_header = sizeof(std::size_t);

    char* buffer;
    BlockHeader* root;

#if DEBUG
    // сумма байт всех запросов
    size_t bytes_allocated;
    // сумма занятых байт
    size_t bytes_used;
    size_t block_counter;
    size_t max_block_count;
    size_t block_size_distribution[LARGE_BUFFER_SIZE + 1] = {0};
#endif

   public:
    LinkedListAllocator() {
        buffer = static_cast<char*>(malloc(LARGE_BUFFER_SIZE));
        // непрямое создание первого блока во весь буффер
        // только в этом языке можно создать обьект без конструктора, лол
        root->prev = root->next = root = reinterpret_cast<BlockHeader*>(buffer);
        root->size = LARGE_BUFFER_SIZE;

#if DEBUG
        bytes_allocated = 0;
        bytes_used = sizeof(BlockHeader);
        block_counter = max_block_count = 1;
        block_size_distribution[LARGE_BUFFER_SIZE] = 1;

        printf(
            "Linked List Allocator construction succesful\n\
root: %p, buffer size: %d\n\n",
            root, LARGE_BUFFER_SIZE);
#endif
    }

    ~LinkedListAllocator() {
#if DEBUG
        printf(
            "Linked Aist Allocator decostructor log\n\
root: addr: %p, size: %lu\n\
bytes left allocated: %lu, bytes left used: %lu\n\
sinle block left? %d, block count: %lu, max block count: %lu\n\
block distribution:\n",
            root, root->size,
            bytes_allocated, bytes_used,
            root->prev == root && root == root->next, block_counter, max_block_count);

        for (size_t size = 0; size < LARGE_BUFFER_SIZE; ++size) {
            size_t count = block_size_distribution[size];
            if (count) {
                printf("size %lu blocks %lu\n", size, count);
            }
        }
#endif
        free(buffer);
    }

    void remove_from_list(BlockHeader* rem) {
        BlockHeader* prev = rem->prev;
        BlockHeader* next = rem->next;
        if (rem == root) {
            if (rem == next) {
                // список из 1 удоляемого элемента - забываем
                root = nullptr;
                return;
            } else {
                // потдержание доступа к списку
                root = next;
            }
        }
        prev->next = next;
        next->prev = prev;
        return;
    }

    char* allocate(size_t size) override {
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
                "No Block of sufficient size error\nBuffer size: %d\n\
                last alloc request/last checked block capacity: %lu/%lu\n",
                LARGE_BUFFER_SIZE, size, cur->size - A_header);
            throw std::bad_alloc();
        }

        // от блока достаточного размера отрезаем новый блок требуемого размера
        if (cur->size >= size + F_hrader + A_header) {
            BlockHeader* cuted_block = reinterpret_cast<BlockHeader*>((char*)cur + cur->size - size - A_header);

#if DEBUG
            bytes_allocated += size;
            bytes_used += size + A_header;
            ++block_counter;
            max_block_count = (block_counter > max_block_count) ? (block_counter) : (max_block_count);
            --block_size_distribution[cur->size];
            ++block_size_distribution[size + A_header];
            ++block_size_distribution[cur->size - size - A_header];
#endif

            cur->size -= (cuted_block->size = size + A_header);
            return cuted_block->data;
        } else {
            remove_from_list(cur);

#if DEBUG
            // не совсем правдо зато без утечек
            bytes_allocated += cur->size - A_header;
            bytes_used += cur->size - F_hrader;
#endif

            return cur->data;
        }
    }

    // Я конечно сделал, но нужно допиливать. Безбожно медленно + по логированию очищается не всё.
    // (вообще не понял почему он не бросает segmentation segmentation falts).
    void deallocate(void* ptr) override {
        //!!! p - адресс на начала данных, не блока
        BlockHeader* to_free = reinterpret_cast<BlockHeader*>((char*)ptr - A_header);
#if DEBUG
        bytes_allocated -= to_free->size - A_header;
#endif
        if (root == nullptr) {
            root = to_free->prev = to_free->next = to_free;
            return;
        }

        BlockHeader *cur = root, *prev_free = nullptr, *next_free = nullptr;
        do {  // запоминаем граничащие свободные блоки
            if ((char*)cur + cur->size == (char*)to_free) {
                prev_free = cur;
            }
            if ((char*)to_free + to_free->size == (char*)cur) {
                next_free = cur;
            }
        } while ((cur = cur->next) != root);

        // далее обработка 4 случаев. (наличие/отсутствие)*(првого/левого) соседа для слияния
        switch ((bool)prev_free << 1 | (bool)next_free) {
        case 0:  // соседей нет => вставляем блок за корнем
            (to_free->prev = root)->next = (to_free->next = root->next)->prev = to_free;
#if DEBUG
            bytes_used -= to_free->size - F_hrader;
#endif
            break;
        case 1:  // только сосед справа => подменяем в списке соседа на себя и присоединяем его
            (to_free->prev = next_free->prev)->next = (to_free->next = next_free->next)->prev = to_free;
            to_free->size += next_free->size;

#if DEBUG
            bytes_used -= to_free->size;
            --block_counter;
            --block_size_distribution[to_free->size];
            --block_size_distribution[next_free->size];
            ++block_size_distribution[to_free->size + next_free->size];
#endif

            break;
        case 2:  // только сосед слева => присоеденяемся к нему
            prev_free->size += to_free->size;

#if DEBUG
            bytes_used -= to_free->size;
            --block_counter;
            --block_size_distribution[prev_free->size];
            --block_size_distribution[to_free->size];
            ++block_size_distribution[to_free->size + prev_free->size];
#endif

            break;
        case 3:  // оба соседа => вырезаем правого и присоденяем себя и правого к левому
            prev_free->size += to_free->size + next_free->size;
            remove_from_list(next_free);

#if DEBUG
            bytes_used -= to_free->size + F_hrader;
            block_counter -= 2;
            --block_size_distribution[prev_free->size];
            --block_size_distribution[to_free->size];
            --block_size_distribution[next_free->size];
            //++block_size_distribution[prev_free->size + to_free->size + next_free->size];
#endif

            break;
        default:
            printf("мы не знаем что это такое...\n");
            throw std::bad_exception();
            break;
        }
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
        // выход на первой отличающийся буквах
        // или кода a = b ='\0' -> ret false
        while (A[0] == B[0] && A[0]) {
            A++;
            B++;
        }
        return A[0] < B[0];
    }
};

char* ReadFromFile(const char* FileName) {
    FILE* File = fopen(FileName, "rb");
    if (File == nullptr) {
        printf("fopen\n");
        printf("Terminal failure: unable to open file \"%s\" for read.\n", FileName);
        return nullptr;
    }

    fseek(File, 0, SEEK_END);
    long long FileSize = ftell(File);
    rewind(File);

    char* ReadBuffer = static_cast<char*>(malloc(FileSize + 1));
    // printf("FileSize = %lld\n", FileSize);

    // Добавил переменную принмающее значение fread, что бы не было warning
    // И сделал проверку на количество реально считанных байт и размер файла
    size_t TotalBytesRead = fread(ReadBuffer, 1, FileSize, File);
    if (TotalBytesRead == (size_t)FileSize) {
        printf("TotalBytesRead and FileSize are the same: %lu\n\n", TotalBytesRead);
    } else {
        printf("WARNING\nTotalBytesRead and FileSize are NOT the same\n");
        printf("TotalBytesRead = %lu and FileSize = %lld\n\n", TotalBytesRead, FileSize);
    }

    ReadBuffer[FileSize] = '\0';
    fclose(File);

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
    // TODO Если хотим работать через Cmake надо че-то делать, потому что cmake запускает программу в совей директории build где нет файла war_en.txt
    // Но каждый раз туда копировать файл как-то странно. Не знаю как решать пока.
    char* ReadBuffer = ReadFromFile("war_en.txt");

    PoolAllocator* poolAllocator = new PoolAllocator();
    auto start2 = std::chrono::high_resolution_clock::now();
    TextMapTest(poolAllocator, strdup(ReadBuffer));
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = end2 - start2;
    printf("PoolAllocator execution time: %f seconds\n\n", duration2.count());
    delete poolAllocator;

    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    auto start1 = std::chrono::high_resolution_clock::now();
    TextMapTest(linkedListAllocator, strdup(ReadBuffer));
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = end1 - start1;
    printf("LinkedListAllocator execution time: %f seconds\n\n", duration1.count());
    delete linkedListAllocator;

    free(ReadBuffer);
    return 0;
}