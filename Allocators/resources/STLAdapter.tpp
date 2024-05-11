// Класс-адаптер для использования с STL контейнерами.

#ifndef STL_ADAPTER
#define STL_ADAPTER

template <class T>
class STLAdapter {
   public:
    typedef T value_type;

    Allocator* allocator = nullptr;

    STLAdapter()
        : allocator(nullptr) {
    }

    STLAdapter(Allocator* ByteAllocator) {
        allocator = ByteAllocator;
    }

    template <class U>
    STLAdapter(const STLAdapter<U>& V)
        : allocator(V.allocator) {
    }

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(allocator->allocate(sizeof(T) * count));
    }

    void deallocate(T* p, size_t count) {
        allocator->deallocate(p);
    }
};

#endif