#include "BinarySearchTree.hh"

int main() {
    BinarySearchTree tree;
    tree.insert(20);
    tree.insert(25);
    tree.insert(15);
    tree.insert(10);
    tree.insert(30);
    tree.display();
    tree.remove(20);
    tree.display();
    tree.remove(25);
    tree.display();
    tree.remove(30);
    tree.display();
    return 0;
}