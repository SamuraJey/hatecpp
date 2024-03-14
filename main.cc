#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>

// #include "pool_allocators.cc"

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024 * 1024
using namespace std;

bool isDelim(char &c)
{
    switch (c)
    {
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

struct Buffer
{
    Buffer *prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

class PoolAllocator : public Allocator
{
    Buffer *some_buffer = nullptr;

  public:
    void createNewBuffer(size_t size)
    {
        Buffer *New = static_cast<Buffer *>(malloc(size + sizeof(Buffer)));
        new (New) Buffer();
        New->prev = some_buffer;
        New->size = size + sizeof(Buffer);
        some_buffer = New;
    }

    PoolAllocator()
    {
        createNewBuffer(BUFFER_SIZE);
    }

    ~PoolAllocator()
    {
        while (some_buffer != nullptr)
        {
            Buffer *prev = some_buffer->prev;
            free(some_buffer);
            some_buffer = prev;
        }
    }

    char *allocate(size_t size) override
    {
        if (some_buffer->size - some_buffer->current < size)
        {
            createNewBuffer(std::max(BUFFER_SIZE, (int)size));
        }

        char *ret = reinterpret_cast<char *>(some_buffer) + some_buffer->current;
        some_buffer->current = some_buffer->current + size;
        return ret;
    }
    void deallocate(void *) override
    {
    }
};

struct BlockHeader
{
    size_t size;
    union {
        struct
        {
            BlockHeader *prev, *next;
        };
        char data[1];
    };
};

class LinkedListAllocator : public Allocator
{
    char *buffer = nullptr;
    BlockHeader *root = nullptr;

  public:
    LinkedListAllocator()
    {
        buffer = static_cast<char *>(malloc(LARGE_BUFFER_SIZE));
        //не проямое создание первого блока во весь буффер
        root = reinterpret_cast<BlockHeader *>(buffer);
        root->size = LARGE_BUFFER_SIZE;
        root->prev = root->next = root;
    }

    void remove_from_free(BlockHeader *cur)
    {
        BlockHeader *prev = cur->prev;
        BlockHeader *next = cur->next;
        prev->next = next;
        next->prev = prev;


        //проверка на единственность в списке
        if (cur == next)
        {
            root = nullptr;
        }
        //проверка не удаляем ли мы вырину-корень списка
        else if (cur == root)
        {
            root = next;
        }
    }


    size_t bytes_allocated = 0;
    char *allocate(size_t size) override
    {
        if (root == nullptr)
        {
            std::cout << "обосрался!!!" << std::endl;
            throw std::bad_alloc();
        }
        BlockHeader *cur = root;

        while (cur->size < size + sizeof(BlockHeader) && cur != root)
        {
            cur = cur->next;
        }
        if(cur->size < size + sizeof(BlockHeader)) {
            std::cout << "total load: " << bytes_allocated << "/" << LARGE_BUFFER_SIZE << " bytes\nlast block cheak: " << (size + sizeof(BlockHeader)) << '/' << cur->size << std::endl;
            throw bad_alloc();
        }
        bytes_allocated += size;
        //от блока досаточного размера отрезаем необходимую часть
        if(cur->size > size + 2 * sizeof(BlockHeader)){
            //будем отдавать именно отрезанный кусок, чтобы не соверать лишних действий со списком свободных блоков
            cur->size -= size + sizeof(BlockHeader);
            BlockHeader* cuted_block = reinterpret_cast<BlockHeader *>(cur->data + (cur->size - size - 2 * sizeof(BlockHeader)));
            cuted_block->size = size;
            return cuted_block->data;
        }
        else{
            remove_from_free(cur);
            return cur->data;
        }

    }
    
    void deallocate(void *p) override
    {
        BlockHeader *cur = reinterpret_cast<BlockHeader *>(p);
        bytes_allocated -= cur->size;
        std::cout << "deallocate: " << cur->size <<std::endl;   
        cur->next = root;
        cur->prev = root->prev;
        root->prev->next = cur;
        root->prev = cur;

    }
};


// PoolAllocator allocator2024;



template <class T>
class CMyAllocator {
public:
    typedef T value_type;

    Allocator* allocator = nullptr;

    CMyAllocator() : allocator(nullptr) {}

    CMyAllocator(Allocator* ByteAllocator) {
        allocator = ByteAllocator;
    }

    template <class U>
    CMyAllocator(const CMyAllocator<U>& V) : allocator(V.allocator) {}

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(allocator->allocate(sizeof(T) * count));
    }

    void deallocate(T* p, size_t count) {
        allocator->deallocate(p);
    }


    
};

bool cmp(pair<const char *, int> First, pair<const char *, int> Second)
{
    return First.second > Second.second;
}

class CStringComparator
{
  public:
    /*
        A < B --> true
    */
    bool operator()(const char *A, const char *B) const
    {
        while (true)
        {
            if (A[0] == B[0])
            {
                // A = B
                if (!A[0])
                    return false;

                A++;
                B++;
            }
            else
            {
                return A[0] < B[0];
            }
        }
    }
};

void TextMapTest(Allocator* allocator)
{
    CMyAllocator<char *> WrapperAllocator(allocator);
    map<const char *, size_t, CStringComparator, CMyAllocator<char *>> Map(WrapperAllocator); // Added parentheses to disambiguate as object declaration
    const char *file_name = "war1.txt";
    FILE *file = fopen(file_name, "rb");

    if (file == nullptr)
    {
        printf("fopen\n");
        printf("Terminal failure: unable to open file \"%s\" for read.\n", file_name);
        return;
    }
    fseek(file, 0, SEEK_END);
    long long file_size = ftell(file);
    rewind(file);

    char *ReadBuffer = static_cast<char *>(malloc(file_size + 1));
    printf("file_size = %lld\n", file_size);
    fread(ReadBuffer, 1, file_size, file);
    ReadBuffer[file_size] = '\0';
    fclose(file);

    char *Word = strtok(ReadBuffer, " \n\t\r");
    while (Word != nullptr)
    {
        Map[Word]++;
        Word = strtok(nullptr, " \n\t\r");
    }

    vector<pair<const char *, int>> SortedWords(Map.begin(), Map.end());
    sort(SortedWords.begin(), SortedWords.end(), cmp);
    int i = 0;
    int num_of_word = 0;

    for (auto Pair : SortedWords)
    {
        if (i++ < 10)
            printf("%s: %d\n", Pair.first, Pair.second);
        num_of_word += Pair.second;
    }
    printf("Total number of words: %d\n", num_of_word);

    free(ReadBuffer);
}

int main()
{
    LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
    PoolAllocator* poolAllocator = new PoolAllocator();


    auto start1 = std::chrono::high_resolution_clock::now();
    TextMapTest(linkedListAllocator);
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = end1 - start1;
    printf("LinkedListAllocator execution time: %f seconds\n", duration1.count());

    auto start2 = std::chrono::high_resolution_clock::now();
    TextMapTest(poolAllocator);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = end2 - start2;
    printf("PoolAllocator execution time: %f seconds\n", duration2.count());


    return 0;
}