cmake_minimum_required(VERSION 3.25)
project(FILP-MO)
set(CMAKE_CXX_STANDARD 23)

add_executable(allocators
    Allocators/main.cc
    Allocators/resources/TextContainer.cc
    Allocators/PoolAllocator.cc
    Allocators/LinkedListAllocator.cc
    Allocators/DescriptorAllocator.cc
    Allocators/BuddyAllocator.cc
    )

add_executable(binary_tree
    Binary\ tree/main.cc
    Binary\ tree/BinarySearchTree.cc
)