#ifndef LIST_TPP
#define LIST_TPP

// List - Circular Double Linked List
// Вынести операции над списком в отдельный темплейтный класс - отличная идея.
// Этот класс можно было бы переиспользовать в каждом аллокаторе.
// Поэтому я добавил в класс дополнительных фич.

template <class E>
class List {
    E* root = nullptr;

   public:
    inline void insert(E* ref, E* new_el) noexcept {
        (new_el->next = ref->next)->prev = new_el;
        (new_el->prev = ref)->next = new_el;
    }

    void put(E* new_el) {
        if (root == nullptr) {
            root = new_el->prev = new_el->next = new_el;
            return;
        }
        insert(root, new_el);
        return;
    }

    inline void replace(E* entry, E* with) noexcept {
        (with->prev = entry->prev)->next = with;
        (with->next = entry->next)->prev = with;
    }

    E* pop() {
        E* rem = root->next;
        if (rem == root) {
            root = nullptr;
        } else {
            (root->next = rem->next)->prev = root;
        }
        return rem;
    }

    // might be depricated
    bool pop(E*& el) {
        el = root->next;
        if (el == root)
            return root = nullptr;
        else
            return (root->next = el->next)->prev = root;
    }

    bool remove(E* entry) {
        E* prev = entry->prev;
        E* next = entry->next;
        if (entry == root && entry == (root = next)) {
            return root = nullptr;
        }
        (prev->next = next)->prev = prev;
        return true;
    }

    inline E* getRoot() noexcept {
        return root;
    }

    inline bool isEmpty() noexcept {
        return root == nullptr;
    }

    inline bool isNotEmpty() noexcept {
        return root;
    }
};

#endif