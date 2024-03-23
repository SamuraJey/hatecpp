cmake_minimum_required(VERSION 3.24)
project(hatecpp)
set(CMAKE_CXX_STANDARD 23)

add_executable(allocators
    main.cc
    # Allocator.hh
    # constants.hh
    # PoolAllocator.hh
    PoolAllocator.cc
    # war_en.txt
    )