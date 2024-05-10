#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "SmartPointer.cc"
#include "SynchroSmartPointer.cc"
// using namespace std;

int main() {
    // Tests for SmartPointer
    std::vector<SmartPointer<std::string>> list;
    list.push_back(SmartPointer<std::string>(new std::string("Hello, world 1")));
    list.push_back(SmartPointer<std::string>(new std::string("Hello, world 2")));
    list.push_back(SmartPointer<std::string>(new std::string("Hello, world 3")));

    for (int i = 0; i < list.size(); i++) {
        printf("list[%d]: %s\n", i, list[i]->c_str());
    }

    // Tests for SynchroSmartPointer
    SynchroSmartPointer<std::string> pointer = SynchroSmartPointer<std::string>(new std::string("Hello, world 1"));
    SynchroSmartPointer<std::string> pointer2 = SynchroSmartPointer<std::string>(pointer);
    SynchroSmartPointer<std::string> pointer3 = SynchroSmartPointer<std::string>(pointer2);
    SynchroSmartPointer<std::string> pointer4 = SynchroSmartPointer<std::string>(pointer);

    printf("pointer 2: %s\n", pointer2->c_str());
    printf("pointer 3: %s\n", pointer3->c_str());
    printf("pointer 4: %s\n", pointer4->c_str());

    return 0;
}