#include "BinarySearchTree.hh"

#include <iostream>

struct BinarySearchTree::Node {
    int data;
    BinarySearchTree::Node* left;
    BinarySearchTree::Node* right;
};

BinarySearchTree::Node* BinarySearchTree::clear(Node* currentNode) {
    if (currentNode != nullptr) {
        clear(currentNode->left);
        clear(currentNode->right);
        delete currentNode;
    }
    return nullptr;
}

BinarySearchTree::Node* BinarySearchTree::insert(int valueToInsert, Node* currentNode) {
    if (currentNode == nullptr) {
        currentNode = new Node;
        currentNode->data = valueToInsert;
        currentNode->left = currentNode->right = nullptr;
        
    } else if (valueToInsert < currentNode->data) // если вставляемое значение меньше текущего узла - идем влево
        currentNode->left = insert(valueToInsert, currentNode->left);
    else if (valueToInsert > currentNode->data) 
        currentNode->right = insert(valueToInsert, currentNode->right);
    return currentNode;
}

BinarySearchTree::Node* BinarySearchTree::findMinimal(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->left == nullptr)
        return currentNode;
    else
        return findMinimal(currentNode->left);
}

BinarySearchTree::Node* BinarySearchTree::findMaximum(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->right == nullptr)
        return currentNode;
    else
        return findMaximum(currentNode->right);
}

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

void BinarySearchTree::inorder(Node* currentNode) {
    if (currentNode == nullptr)
        return;
    inorder(currentNode->left);
    std::cout << currentNode->data << " ";
    inorder(currentNode->right);
}

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

void BinarySearchTree::insert(int x) {
    root = insert(x, root);
}

void BinarySearchTree::remove(int x) {
    root = remove(x, root);
}

void BinarySearchTree::display() {
    inorder(root);
    std::cout << std::endl;
}

void BinarySearchTree::search(int x) {
    root = find(root, x);
}
