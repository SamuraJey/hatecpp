#include <iostream>
#include <thread>

#include "SynchroSmartPointer.cc"

int main() {
    // Create an integer and wrap it in a SynchroSmartPointer
    // int* num = new int(5);
    SynchroSmartPointer<int> ptr1 = SynchroSmartPointer<int>::makePointer(new int(5));
    SynchroSmartPointer<int> ptr2 = ptr1;
    SynchroSmartPointer<int> ptr3 = ptr1;
    //SynchroSmartPointer<int> ptr3(ptr1);
    SynchroSmartPointer<int> ptr4 = ptr1;
    SynchroSmartPointer<int> ptr5 = ptr1;
    SynchroSmartPointer<int> ptr6 = ptr1;
    SynchroSmartPointer<int> ptr7 = ptr1;

    std::thread t1([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr1)++;
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr2)++;
        }
    });

    std::thread t3([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr3)++;
        }
    });

    std::thread t4([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr4)++;
        }
    });

    std::thread t5([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr5)++;
        }
    });

    std::thread t6([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr6)++;
        }
    });

    std::thread t7([&]() {
        for (int i = 0; i < 100; i++) {
            (*ptr7)++;
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    

    std::cout << "Final value: " << *ptr1 << std::endl;

    return 0;
}