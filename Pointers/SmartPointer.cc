// макрос оборачивающий функцию printf. Если выключить - припроцессор выпилит все дебаг принты из изходника ещё перед компиляцией
#if true
#define debug(format, ...) printf(format __VA_OPT__(, ) __VA_ARGS__)
#else
#define debug(...)
#endif

template <class T>
class SmartPointer {
    T* resource;
    size_t* count;

   public:
    template <class V>
    static SmartPointer<V> makePointer(V* value) {
        return SmartPointer<V>(value);
    }

    // констуктор оборачивающий простую ссылку. пометка explicit предотварщает неявные вызов конструктора.
    explicit SmartPointer(T* value) {
        if (value == nullptr) {
            // TODO нормально ли такой exception?
            // можно использовать std::invalid_argument
            throw std::logic_error("bad value\n");
        }
        this->resource = value;
        this->count = new size_t(1);
        debug("constructor: %p, recource: %p, count: %llu\n", this, this->resource, *(this->count));
    }

    // copy constructor
    SmartPointer(const SmartPointer& other) {
        debug("copy constructor %p from %p\n", this, &other);
        assign(other);
    }

    ~SmartPointer() {
        debug("destructor: %p, recource: %p, count: %llu\n", this, this->resource, (*this->count));
        revoke();
    }

    T& operator->() {
        debug("operator-> %p\n", this);
        return *resource;
    }

    T& operator*() {
        debug("operator* %p\n", this);
        return *resource;
    }

    SmartPointer& operator=(const SmartPointer& other) {
        debug("operator= %p, recource: %p, count: %llu\n", this, this->resource, (*this->count));
        revoke();
        assign(other);
        return *this;
    }

   private:
    // assigne smartptr to a resource
    void assign(const SmartPointer& other) {
        this->resource = other.resource;
        this->count = other.count;
        (*this->count)++;
        debug("method assign: %p, new recource: %p, count: %llu\n", this, this->resource, (*this->count));
    }

    // retrive the smartptr's assigment from the resource
    void revoke() {
        switch (*this->count) {
        case 0:
            throw std::runtime_error("that should not supposed to happen");
            break;

        case 1:
            debug("delete resource\n");
            delete this->resource;
        default:
            (*this->count)--;
            debug("method revoke: %p, recource: %p, count: %llu\n", this, this->resource, *this->count);
            break;
        }
    }
};