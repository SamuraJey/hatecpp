template <class T>
class SmartPointer {
    T* value;
    size_t* count;

   public:
    template <class V>
    static SmartPointer<V> makePointer(V* value) {
        return SmartPointer<V>(value);
    }

    explicit SmartPointer(T* value) {
        if (value == nullptr) {
            // TODO нормально ли такой exception?
            throw std::logic_error("bad value\n");
        }
        this->value = value;
        this->count = new size_t(1);
        printf("Method: constructor, count: %llu, ref: %d\n", static_cast<size_t>(*this->count), this);
    }

    SmartPointer(const SmartPointer& other) {
        copyObject(other);
    }

    ~SmartPointer() {
        deleteObject();
    }

    T* operator->() {
        return value;
    }

    T& operator*() {
        return *value;
    }

    SmartPointer& operator=(const SmartPointer& other) {
        deleteObject();
        copyObject(other);
        return *this;
    }

   private:
    void copyObject(const SmartPointer& other) {
        this->value = other.value;
        this->count = other.count;
        (*this->count)++;
        printf("Method: copyObject, count: %llu, ref: %d\n", static_cast<size_t>(*this->count), this);
    }

    void deleteObject() {
        if (*this->count > 0) {
            (*this->count)--;
            printf("Method: deleteObject, count: %llu, ref: %d\n", static_cast<size_t>(*this->count), this);
        }
        if (*(this->count) == 0) {
            delete this->value;
        }
    }
};