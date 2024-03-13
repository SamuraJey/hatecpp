#include <stdio.h>
#include <algorithm>
// #include <chrono>
#include <cstdint>
// #include <cstring>
// #include <iostream>
#include <map>

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024 * 10
// using namespace std;

struct Buffer
{
    Buffer *prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

class PoolAllocator
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

    char *allocate(size_t size)
    {
        if (some_buffer->size - some_buffer->current < size)
        {
            createNewBuffer(std::max(BUFFER_SIZE, (int)size));
        }

        char *ret = reinterpret_cast<char *>(some_buffer) + some_buffer->current;
        some_buffer->current = some_buffer->current + size;
        return ret;
    }
    void deallocate(void *)
    {
    }
};

PoolAllocator poolAllocator;

template <class T> class CPoolAllocator
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
        return reinterpret_cast<T *>(poolAllocator.allocate(sizeof(T) * Count));
    }

    void deallocate(T *V, size_t Count)
    {
        poolAllocator.deallocate(V);
    }
};


