#include "BinarySearchTree.hh"

#include <iostream>

struct BinarySearchTree::Node {
    int data;
    BinarySearchTree::Node* left;
    BinarySearchTree::Node* right;
};

/**
 * Recursively clears the binary search tree starting from the given node.
 * 
 * @param currentNode The current node to start clearing from.
 * @return nullptr after clearing the tree.
 */
BinarySearchTree::Node* BinarySearchTree::clear(Node* currentNode) {
    if (currentNode != nullptr) {
        clear(currentNode->left);
        clear(currentNode->right);
        delete currentNode;
    }
    return nullptr;
}

/**
 * Inserts a new node with the given value into the binary search tree.
 * If the tree is empty, a new root node is created.
 * If the value is less than the current node's data, it is inserted to the left of the current node.
 * If the value is greater than the current node's data, it is inserted to the right of the current node.
 * 
 * @param valueToInsert The value to be inserted into the tree.
 * @param currentNode The current node being evaluated during the insertion process.
 * @return The updated node structure after the insertion.
 */
BinarySearchTree::Node* BinarySearchTree::insert(int valueToInsert, Node* currentNode) {
    if (currentNode == nullptr) {
        currentNode = new Node;
        currentNode->data = valueToInsert;
        currentNode->left = currentNode->right = nullptr;

    } else if (valueToInsert < currentNode->data)  // если вставляемое значение меньше текущего узла - идем влево
        currentNode->left = insert(valueToInsert, currentNode->left);
    else if (valueToInsert > currentNode->data)
        currentNode->right = insert(valueToInsert, currentNode->right);
    return currentNode;
}

/**
 * Finds the node with the minimal value in a binary search tree recursively.
 *
 * @param currentNode The current node being checked.
 * @return The node with the minimal value, or nullptr if the tree is empty.
 */
BinarySearchTree::Node* BinarySearchTree::findMinimal(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->left == nullptr)
        return currentNode;
    else
        return findMinimal(currentNode->left);
}

/**
 * Finds the node with the maximal value in a binary search tree recursively.
 *
 * @param currentNode The current node being checked.
 * @return The node with the maximal value, or nullptr if the tree is empty.
 */
BinarySearchTree::Node* BinarySearchTree::findMaximum(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->right == nullptr)
        return currentNode;
    else
        return findMaximum(currentNode->right);
}

/**
 * Removes a node with the specified value from the binary search tree.
 * 
 * @param valueToRemove The value to be removed from the tree.
 * @param currentNode The current node being checked.
 * @return The updated root node of the binary search tree.
 */
BinarySearchTree::Node* BinarySearchTree::remove(int valueToRemove, Node* currentNode) {
    Node* temp;
    if (currentNode == nullptr)
        return nullptr;

    if (valueToRemove < currentNode->data)
        currentNode->left = remove(valueToRemove, currentNode->left);
    else if (valueToRemove > currentNode->data)
        currentNode->right = remove(valueToRemove, currentNode->right);
    else if (currentNode->left && currentNode->right) {
        temp = findMinimal(currentNode->right);
        currentNode->data = temp->data;
        currentNode->right = remove(currentNode->data, currentNode->right);
    } else {
        temp = currentNode;
        if (currentNode->left == nullptr)
            currentNode = currentNode->right;
        else if (currentNode->right == nullptr)
            currentNode = currentNode->left;
        delete temp;
    }

    return currentNode;
}

/**
 * Prints the elements of the binary search tree in inorder traversal.
 * 
 * @param currentNode The current node being visited.
 */
void BinarySearchTree::inorder(Node* currentNode) {
    if (currentNode == nullptr)
        return;
    inorder(currentNode->left);
    std::cout << currentNode->data << " ";
    inorder(currentNode->right);
}

/**
 * Recursively finds a node with the specified value in the binary search tree.
 * 
 * @param currentNode The current node being checked.
 * @param valueToFind The value to find in the binary search tree.
 * @return A pointer to the node with the specified value, or nullptr if not found.
 */
BinarySearchTree::Node* BinarySearchTree::find(Node* currentNode, int valueToFind) {
    if (currentNode == nullptr)
        return nullptr;

    if (valueToFind < currentNode->data)
        return find(currentNode->left, valueToFind);
    else if (valueToFind > currentNode->data)
        return find(currentNode->right, valueToFind);
    else
        return currentNode;
}

BinarySearchTree::BinarySearchTree() {
    root = nullptr;
}

BinarySearchTree::~BinarySearchTree() {
    root = clear(root);
}

/**
 * Inserts a new element into the binary search tree.
 *
 * @param x The value to be inserted.
 */
void BinarySearchTree::insert(int x) {
    root = insert(x, root);
}

/**
 * Removes a node with the specified value from the binary search tree.
 *
 * @param x The value to be removed.
 */
void BinarySearchTree::remove(int x) {
    root = remove(x, root);
}

/**
 * Displays the elements of the binary search tree in an inorder traversal.
 * The elements are printed to the standard output, followed by a newline.
 */
void BinarySearchTree::display() {
    inorder(root);
    std::cout << std::endl;
}

/**
 * Searches for a given value in the binary search tree.
 * @param x The value to search for.
 */
void BinarySearchTree::search(int x) {
    root = find(root, x);
}
