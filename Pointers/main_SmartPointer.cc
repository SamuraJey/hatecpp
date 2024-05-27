#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "SmartPointer.cc"
#include "SynchroSmartPointer.cc"
// using namespace std;

int main() {
    // "Wrapper constructors"
    std::string* ptr0 = new std::string("Basic String");

    SmartPointer ptr1(new std::string("Hello, world"));
    SmartPointer ptr2(new std::string("String number 3"));
    // implicit copy constructor induced by assigment semantics
    SmartPointer ptr3 = ptr1;
    // assigment operator
    ptr1 = ptr2;

    debug("\nPushing to a vector\n\n");
    // у вектора (почему-то) размер по умолчанию 1. При добавлении 3 элементов он дважды пересодаст массив 1->2->4. И кучу раз скопирует элементы
    std::vector<SmartPointer<std::string>> list;
    list.push_back(ptr1);
    list.push_back(ptr2);
    list.push_back(ptr3);

    debug("\nReading from vector\n\n");

    for (int i = 0; i < list.size(); i++) {
        printf("list[%d]: %s\n", i, (*list[i]).c_str());
    }
    return 0;
}