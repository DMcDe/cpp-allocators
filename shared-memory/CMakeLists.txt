cmake_minimum_required(VERSION 3.10)

project(SharedAllocator)

set(CMAKE_BUILD_TYPE Debug)

add_executable(SharedAllocator shmem-allocator.cpp shmem-test.cpp)