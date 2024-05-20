// Класс-адаптер для использования с STL контейнерами.

#ifndef STL_ADAPTER
#define STL_ADAPTER

/**
 * @brief A class template that serves as an adapter for using custom allocators with STL containers.
 * 
 * @tparam T The type of elements stored in the container.
 */
template <class T>
class STLAdapter {
   public:
    typedef T value_type;

    Allocator* allocator = nullptr;

    /**
     * @brief Default constructor for STLAdapter.
     * 
     * Initializes the allocator pointer to nullptr.
     */
    STLAdapter()
        : allocator(nullptr) {
    }

    /**
     * @brief Constructor for STLAdapter that takes an allocator as a parameter.
     * 
     * @param ByteAllocator A pointer to the allocator to be used.
     */
    STLAdapter(Allocator* ByteAllocator) {
        allocator = ByteAllocator;
    }

    /**
     * @brief Copy constructor for STLAdapter.
     * 
     * @tparam U The type of elements stored in the source adapter.
     * @param V The source adapter to be copied.
     */
    template <class U>
    STLAdapter(const STLAdapter<U>& V)
        : allocator(V.allocator) {
    }

    /**
     * @brief Allocates memory for a specified number of objects of type T.
     * 
     * @param count The number of objects to allocate memory for.
     * @return T* A pointer to the allocated memory.
     */
    T* allocate(size_t count) {
        return reinterpret_cast<T*>(allocator->allocate(sizeof(T) * count));
    }

    /**
     * @brief Deallocates memory previously allocated by the allocate function.
     * 
     * @param p A pointer to the memory to deallocate.
     * @param count The number of objects that were previously allocated.
     */
    void deallocate(T* p, size_t count) {
        allocator->deallocate(p);
    }
};

#endif