// макрос оборачивающий функцию printf. Если выключить - припроцессор выпилит все дебаг принты из изходника ещё перед компиляцией
// todo change printf to something more modern??? 
#if true
#define debug(format, ...) printf(format __VA_OPT__(, ) __VA_ARGS__)
#else
#define debug(...)
#endif

/**
 * @brief A smart pointer class that manages the lifetime of a dynamically allocated resource.
 * 
 * @tparam T The type of the resource being managed.
 */
template <class T>
class SmartPointer {
    T* resource; // Pointer to the resource
    size_t* count; // Pointer to the reference count

   public:
    /**
     * @brief Creates a new SmartPointer object from a raw pointer.
     * 
     * @tparam V The type of the resource being managed.
     * @param value A raw pointer to the resource.
     * @return SmartPointer<V> A new SmartPointer object.
     */
    template <class V>
    static SmartPointer<V> makePointer(V* value) {
        return SmartPointer<V>(value);
    }

    /**
     * @brief Constructs a SmartPointer object from a raw pointer.
     * 
     * @param value A raw pointer to the resource.
     */
    explicit SmartPointer(T* value) {
        if (value == nullptr) {
            throw std::logic_error("bad value\n");
        }
        this->resource = value;
        this->count = new size_t(1);
        debug("constructor: %p, recource: %p, count: %llu\n", this, this->resource, *(this->count));
    }

    /**
     * @brief Copy constructor.
     * 
     * @param other The SmartPointer object to copy from.
     */
    SmartPointer(const SmartPointer& other) {
        debug("copy constructor %p from %p\n", this, &other);
        assign(other);
    }

    /**
     * @brief Destructor.
     */
    ~SmartPointer() {
        debug("destructor: %p, recource: %p, count: %llu\n", this, this->resource, (*this->count));
        revoke();
    }

    /**
     * @brief Overloaded arrow operator.
     * 
     * @return T& A reference to the resource.
     */
    T& operator->() {
        debug("operator-> %p\n", this);
        return *resource;
    }

    /**
     * @brief Overloaded dereference operator.
     * 
     * @return T& A reference to the resource.
     */
    T& operator*() {
        debug("operator* %p\n", this);
        return *resource;
    }

    /**
     * @brief Assignment operator.
     * 
     * @param other The SmartPointer object to assign from.
     * @return SmartPointer& A reference to the assigned SmartPointer object.
     */
    SmartPointer& operator=(const SmartPointer& other) {
        debug("operator= %p, recource: %p, count: %llu\n", this, this->resource, (*this->count));
        revoke();
        assign(other);
        return *this;
    }

   private:
    /**
     * @brief Assigns the resource and reference count from another SmartPointer object.
     * 
     * @param other The SmartPointer object to assign from.
     */
    void assign(const SmartPointer& other) {
        this->resource = other.resource;
        this->count = other.count;
        (*this->count)++;
        debug("method assign: %p, new recource: %p, count: %llu\n", this, this->resource, (*this->count));
    }

    /**
     * @brief Decrements the reference count and deletes the resource if necessary.
     */
    void revoke() {
        switch (*this->count) {
        case 0:
            throw std::runtime_error("that should not supposed to happen");
            break;
        case 1:
            debug("delete resource\n");
            delete this->resource;
            delete this->count; 
            this->resource = nullptr;
            this->count = nullptr;
            break;
        default:
            (*this->count)--;
            debug("method revoke: %p, recource: %p, count: %llu\n", this, this->resource, *this->count);
            break;
        }
    }
};