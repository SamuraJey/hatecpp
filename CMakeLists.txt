cmake_minimum_required(VERSION 3.25)
project(hatecpp)
set(CMAKE_CXX_STANDARD 23)

add_executable(allocators
    main.cc
    TextContainer.cc
    Allocators/PoolAllocator.cc
    Allocators/LinkedListAllocator.cc
    Allocators/DescriptorAllocator.cc
    Allocators/BinaryAllocator.cc
    )