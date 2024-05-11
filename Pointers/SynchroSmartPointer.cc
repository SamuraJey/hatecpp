template <class T>
class SynchroSmartPointer {
    struct SynchroObject {
        size_t count;
        std::mutex locker;
    };
    T* value;
    SynchroObject* synchroCount;

   public:
    template <class V>
    static SynchroSmartPointer<V> makePointer(V* value) {
        return SynchroSmartPointer<V>(value);
    }

    explicit SynchroSmartPointer(T* value) {
        if (value == nullptr) {
            // TODO нормально ли такой exception?
            throw std::logic_error("bad value\n");
        }
        this->value = value;
        this->synchroCount = new SynchroObject();
        this->synchroCount->count = 1;
        printf("Method: constructor, count: %llu, ref: %d\n", static_cast<size_t>(this->synchroCount->count), this);
    }

    SynchroSmartPointer(const SynchroSmartPointer& other) {
        this->synchroCount = new SynchroObject();
        lock();
        copyObject(other);
        unlock();
    }

    ~SynchroSmartPointer() {
        lock();
        deleteObject();
        unlock();
    }

    T* operator->() {
        // while (!isLock()) {
        //     // Wait until the lock is released
        // }
        // Изначально там был пустой while() и я подумал, что надо ждать пока мютекс освободится, но вроде как не надо.
        return value;
    }

    T& operator*() {
        return *value;
    }

    SynchroSmartPointer& operator=(const SynchroSmartPointer& other) {
        lock();
        deleteObject();
        copyObject(other);
        unlock();
        return *this;
    }

   private:
    void lock() {
        this->synchroCount->locker.lock();
    }

    void unlock() {
        this->synchroCount->locker.unlock();
    }

    bool isLock() {
        return this->synchroCount->locker.try_lock();
    }

    void copyObject(const SynchroSmartPointer& other) {
        this->value = other.value;
        if (this->synchroCount == nullptr) {
            this->synchroCount = new SynchroObject();
        }
        this->synchroCount->count = other.synchroCount->count;
        this->synchroCount->count++;
        printf("Method: copyObject, count: %llu, ref: %d\n", static_cast<size_t>(this->synchroCount->count), this);
    }

    void deleteObject() {
        if (this->synchroCount->count > 0) {
            this->synchroCount->count--;
            printf("Method: deleteObject, count: %llu, ref: %d\n", static_cast<size_t>(this->synchroCount->count), this);
        }
        if (this->synchroCount->count == 0) {
            delete this->value;
        }
    }
};