#ifndef BINARY_TREE_H
#define BINARY_TREE_H

/**
 * @class BinarySearchTree
 * @brief Represents a binary search tree data structure.
 *
 * The BinarySearchTree class provides methods to insert, remove, display, and search elements in a binary search tree.
 * It also supports finding the minimum and maximum values in the tree.
 * @note Non functional-styled implementation. You may consider writing a functional-styled implementation.
 */
class BinarySearchTree {
   public:
    BinarySearchTree();
    ~BinarySearchTree();
    void insert(int x);
    void remove(int x);
    void display();
    void search(int x);

   private:
    struct Node;
    Node* root;
    Node* clear(Node* currentNode);
    Node* insert(int valueToInsert, Node* currentNode);
    Node* findMinimal(Node* currentNode);
    Node* findMaximum(Node* currentNode);
    Node* remove(int valueToRemove, Node* currentNode);
    Node* find(Node* currentNode, int valueToFind);
    void inorder(Node* currentNode);
};

#endif  // !BINARY_TREE_H