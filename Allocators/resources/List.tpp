#ifndef LIST_TPP
#define LIST_TPP

/**
 * @brief Circular Double Linked List template class.
 * 
 * This class represents a circular double linked list.
 * It provides operations for inserting, replacing, removing, and accessing elements in the list.
 * The class can be reused in different allocators.
 * 
 * @tparam E The type of elements stored in the list.
 */
template <class E>
class List {
    E* root = nullptr;

   public:
    /**
     * @brief Inserts a new element after a given reference element.
     * 
     * @param ref The reference element after which the new element should be inserted.
     * @param new_el The new element to be inserted.
     */
    inline void insert(E* ref, E* new_el) noexcept {
        (new_el->next = ref->next)->prev = new_el;
        (new_el->prev = ref)->next = new_el;
    }

    /**
     * @brief Puts a new element at the beginning of the list.
     * 
     * If the list is empty, the new element becomes the root element.
     * Otherwise, the new element is inserted after the root element.
     * 
     * @param new_el The new element to be added to the list.
     */
    void put(E* new_el) {
        if (root == nullptr) {
            root = new_el->prev = new_el->next = new_el;
            return;
        }
        insert(root, new_el);
        return;
    }

    /**
     * @brief Replaces an existing element with a new element.
     * 
     * @param entry The element to be replaced.
     * @param with The new element to replace the existing element.
     */
    inline void replace(E* entry, E* with) noexcept {
        (with->prev = entry->prev)->next = with;
        (with->next = entry->next)->prev = with;
    }

    /**
     * @brief Removes and returns the root element of the list.
     * 
     * If the list becomes empty after removing the root element, the root is set to nullptr.
     * 
     * @return The removed root element.
     */
    E* pop() {
        E* rem = root->next;
        if (rem == root) {
            root = nullptr;
        } else {
            (root->next = rem->next)->prev = root;
        }
        return rem;
    }

    /**
     * @brief Removes and returns the root element of the list.
     * 
     * This function is deprecated and might not be used.
     * 
     * @param el A reference to store the removed root element.
     * @return True if the list is not empty after removing the root element, false otherwise.
     */
    bool pop(E*& el) {
        el = root->next;
        if (el == root)
            return root = nullptr;
        else
            return (root->next = el->next)->prev = root;
    }

    /**
     * @brief Removes a specific element from the list.
     * 
     * @param entry The element to be removed.
     * @return True if the element was successfully removed, false otherwise.
     */
    bool remove(E* entry) {
        E* prev = entry->prev;
        E* next = entry->next;
        if (entry == root && entry == (root = next)) {
            return root = nullptr;
        }
        (prev->next = next)->prev = prev;
        return true;
    }

    /**
     * @brief Returns the root element of the list.
     * 
     * @return The root element of the list.
     */
    inline E* getRoot() noexcept {
        return root;
    }

    /**
     * @brief Checks if the list is empty.
     * 
     * @return True if the list is empty, false otherwise.
     */
    inline bool isEmpty() noexcept {
        return root == nullptr;
    }

    /**
     * @brief Checks if the list is not empty.
     * 
     * @return True if the list is not empty, false otherwise.
     */
    inline bool isNotEmpty() noexcept {
        return root;
    }
};

#endif