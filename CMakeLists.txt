cmake_minimum_required(VERSION 3.25)
project(FILP-MO)
set(CMAKE_CXX_STANDARD 23)

add_executable(allocators
    main.cc
    Allocators/PoolAllocator.cc
    Allocators/LinkedListAllocator.cc
    TextContainer.cc
    Allocators/DescriptorAllocator.cc
    )

add_executable(binary_tree
    Binary\ tree/main.cc
    Binary\ tree/BinarySearchTree.cc
)