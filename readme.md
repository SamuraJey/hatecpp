# FILP-MO
Repository for tasks from the course "Functional and logical programming" 
Completed tasks:
- Text Map - count number of words in the text
- Pool Allocator
- Linked List Allocator
- Descriptor Allocator
- Buddy Allocator (Binary Allocator)
- Binary Search Tree
- Smart Pointer

Building with CMake, C++23 standard is used.
Buddy allocator is using assembly instruction "bsr" which is not supported by all processors, so it may not work on ARM processors.\
SynchroSmartPointer is not working properly, it is not recommended to use it. But feel free to try fix it and make a pull request.


## Authors
- [Dmitrii Chernyavskii](https://github.com/Jamaske)
- [Sergei Zaremba](https://github.com/SamuraJey)
