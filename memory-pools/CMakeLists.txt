cmake_minimum_required(VERSION 3.10)

project(MemoryPool)

set(CMAKE_BUILD_TYPE Debug)

add_executable(MemoryPool memory-pool.cpp pool-test.cpp)