#include <mutex>
#include <stdexcept>
template <class T>
class SynchroSmartPointer {
    struct SynchroObject {
        size_t count;
        std::mutex locker;
    };
    T* value;
    SynchroObject* descriptor;

   public:
    template <class V>
    static SynchroSmartPointer<V> makePointer(V* value) {
        return SynchroSmartPointer<V>(value);
    }

    explicit SynchroSmartPointer(T* value) {
        if (value == nullptr) {
            // TODO нормально ли такой exception?
            throw std::logic_error("bad value - nullptr\n");
        }
        this->value = value;
        this->descriptor = new SynchroObject();
        this->descriptor->count = 1;
        printf("Method: constructor, count: %llu, ref: %p\n", static_cast<size_t>(this->descriptor->count), this);
    }

    SynchroSmartPointer(const SynchroSmartPointer& other) {
        // this->descriptor = new SynchroObject();
        //  lock();
        copyObject(other);
        // unlock();
    }

    ~SynchroSmartPointer() {
        // lock();
        deleteObject();
        // unlock();
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
        deleteObject();
        copyObject(other);
        return *this;
    }

   private:
    void lock() {
        this->descriptor->locker.lock();
    }

    void unlock() {
        this->descriptor->locker.unlock();
    }

    bool isLock() {
        return this->descriptor->locker.try_lock();
    }

    void copyObject(const SynchroSmartPointer& other) {
        this->value = other.value;
        this->descriptor = other.descriptor;
        this->descriptor->locker.lock();
        this->descriptor->count++;
        printf("Method: copyObject, count: %llu, ref: %p\n", static_cast<size_t>(this->descriptor->count), this);
        this->descriptor->locker.unlock();
    }

    void deleteObject() {
        this->descriptor->locker.lock();
        if (this->descriptor->count > 0) {
            this->descriptor->count--;
            printf("Method: deleteObject, count: %llu, ref: %p\n", static_cast<size_t>(this->descriptor->count), this);
        } else {
            throw std::runtime_error("that should not supposed to happen");
        }

        if (this->descriptor->count == 0) {
            delete this->descriptor;
            delete this->value;
        } else {
            this->descriptor->locker.unlock();
        }
    }
};