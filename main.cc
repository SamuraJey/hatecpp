#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024 * 1024 * 3
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
    size_t bytes_used = sizeof(BlockHeader);  // сумма занятых байт

   public:
    LinkedListAllocator() {
        buffer = static_cast<char*>(malloc(LARGE_BUFFER_SIZE));
        // непрямое создание первого блока во весь буффер
        // только в этом языке можно создать обьект без конструктора, лол
        root = reinterpret_cast<BlockHeader*>(buffer);
        root->size = LARGE_BUFFER_SIZE;
        root->prev = root->next = root;
        printf("LLAllocator construction succesful\nroot: %p, buffer size: %d\n",
               root, LARGE_BUFFER_SIZE);
    }

    ~LinkedListAllocator() {
        printf("LLAllocator decostructor log\nroot: %p, size: %d, sinle block left? %d\nbytes left allocated: %d, bytes left used: %d\n",
               root, root->size, root == root->next, bytes_allocated, bytes_used);
        free(buffer);
    }

    void remove_from_free(BlockHeader* rem) {
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
        while (cur->size < size + sizeof(size_t) && cur != root) {
            cur = cur->next;
        }
        if (cur->size < size + sizeof(size_t)) {
            printf(
                "No Block of sufficient size error\nBuffer size: %d\nallocated bytes total: %d\n\
                bytes in uses total: \nlast alloc request/last checked block capacity: %d/%d\n",
                LARGE_BUFFER_SIZE, bytes_allocated, bytes_used, size, cur->size - sizeof(size_t));
            throw std::bad_alloc();
        }

        bytes_allocated += size;
        // от блока достаточного размера отрезаем необходимую часть
        if (cur->size >= size + sizeof(BlockHeader) + sizeof(size_t)) {
            // будем отдавать новый кусок, чтобы не соверать лишних действий со списком свободных блоков
            BlockHeader* cuted_block = reinterpret_cast<BlockHeader*>((char*)cur + cur->size - size - sizeof(size_t));

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

    // Я конечно сделал, но нужно допиливать. Безбожно медленно + по логированию очищается не всё.
    // (вообще не понял почему он не бросает segmentation segmentation falts).
    void deallocate(void* ptr) override {
        //!!! p - адресс на начала данных, не блока
        BlockHeader* to_free = reinterpret_cast<BlockHeader*>((char*)ptr - sizeof(std::size_t));

        bytes_allocated -= to_free->size - sizeof(std::size_t);  // накапливается ошибка подсчёта
        bytes_used -= to_free->size - sizeof(BlockHeader);

        if (root == nullptr) {
            root = to_free->prev = to_free->next = to_free;
            return;
        }

        BlockHeader *cur = root, *prev_free = nullptr, *next_free = nullptr;
        do {  // запоминаем граничащие свободные блоки
            if ((char*)cur + cur->size == (char*)to_free) {
                prev_free = cur;
            } else if ((char*)to_free + to_free->size == (char*)cur) {
                next_free = cur;
            }
        } while ((cur = cur->next) != root);

        // далее обработка 4 случаев. (наличие/отсутствие)*(првого/левого) соседа для слияния
        switch ((bool)prev_free << 1 | (bool)next_free) {
        case 0:  // соседей нет => вставляем блок за корнем
            (to_free->prev = root)->next = (to_free->next = root->next)->prev = to_free;
            break;
        case 1:  // только сосед справа => подменяем в списке соседа на себя и присоединяем его
            (to_free->prev = next_free->prev)->next = (to_free->next = next_free->next)->prev = to_free;
            to_free->size += next_free->size;
            break;
        case 2:  // только сосед слева => присоеденяемся к нему
            prev_free->size += to_free->size;
            break;
        case 3:  // оба соседа => вырезаем правого и присоденяем себя и правого к левому
            prev_free->size += to_free->size + next_free->size;
            remove_from_free(next_free);
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
    char* ReadBuffer = ReadFromFile("war_en.txt");

    PoolAllocator* poolAllocator = new PoolAllocator();
    auto start2 = std::chrono::high_resolution_clock::now();
    TextMapTest(poolAllocator, strdup(ReadBuffer));
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = end2 - start2;
    printf("PoolAllocator execution time: %f seconds\n\n", duration2.count());

    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    auto start1 = std::chrono::high_resolution_clock::now();
    TextMapTest(linkedListAllocator, strdup(ReadBuffer));
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = end1 - start1;
    printf("LinkedListAllocator execution time: %f seconds\n\n", duration1.count());

    free(ReadBuffer);
    delete linkedListAllocator;
    delete poolAllocator;
    return 0;
}