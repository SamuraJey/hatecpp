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