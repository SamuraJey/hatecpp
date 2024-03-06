#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024 * 10
using namespace std;

struct Buffer
{
    Buffer *prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
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

class LinkedListAllocator
{
    char *buffer = nullptr;
    BlockHeader *root = nullptr;

  public:
    LinkedListAllocator()
    {
        buffer = static_cast<char *>(malloc(LARGE_BUFFER_SIZE));
        root = reinterpret_cast<BlockHeader *>(buffer);
        root->size = LARGE_BUFFER_SIZE;
        root->prev = root->next = root;
    }

    void remove(BlockHeader *cur)
    {
        BlockHeader *prev = cur->prev;
        BlockHeader *next = cur->next;
        prev->next = next;
        next->prev = prev;
        if (next == prev)
        {
            root = nullptr;
        }
        else if (root == cur)
        {
            root = next;
        }
    }

    char *allocate(size_t size)
    {
        if (root == nullptr)
        {
            throw bad_alloc();
        }
        BlockHeader *cur = root;
        while (cur->size < size + sizeof(BlockHeader) && cur != root)
        {
            cur = cur->next;
        }
    }
};

class Allocator
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

    Allocator()
    {
        createNewBuffer(BUFFER_SIZE);
    }

    ~Allocator()
    {
        while (some_buffer != nullptr)
        {
            Buffer *prev = some_buffer->prev;
            free(some_buffer);
            some_buffer = prev;
        }
    }

    char *allocate(size_t size)
    {
        if (some_buffer->size - some_buffer->current < size)
        {
            createNewBuffer(max(BUFFER_SIZE, (int)size));
        }

        char *ret = reinterpret_cast<char *>(some_buffer) + some_buffer->current;
        some_buffer->current = some_buffer->current + size;
        return ret;
    }
    void deallocate(void *)
    {
    }
};

Allocator allocator2024;

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

template <class T> class CMyAllocator
{
  public:
    typedef T value_type;

    CMyAllocator()
    {
    }

    template <class U> CMyAllocator(const CMyAllocator<U> &V)
    {
    }

    T *allocate(size_t Count)
    {
        return reinterpret_cast<T *>(allocator2024.allocate(sizeof(T) * Count));
    }

    void deallocate(T *V, size_t Count)
    {
        allocator2024.deallocate(V);
    }
};

bool cmp(pair<const char *, int> First, pair<const char *, int> Second)
{
    return First.second > Second.second;
}

void TextMapTest()
{
    map<const char *, size_t, CStringComparator, CMyAllocator<char *>> Map;
    const char *file_name = "test.txt";
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

    for (auto Pair : SortedWords)
    {
        printf("%s: %d\n", Pair.first, Pair.second);
    }

    free(ReadBuffer);
}

#include <chrono>

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    TextMapTest();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    printf("Execution time: %f seconds\n", duration.count());
    return 0;
}