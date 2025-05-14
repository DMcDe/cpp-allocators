#include "shmem-allocator.h"
#include <cstring>
#include <iostream>

#ifndef MEMORYSIZE
#define MEMORYSIZE 2048
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE 256
#endif

int main() {
    const char* file_key = "/home/dmcd/misc/allocators/README.md";
    const char* sem_name = "/semaphore";
    SharedAllocator alloc(file_key, sem_name, MEMORYSIZE);

    std::cout << "Allocating a 256B block of memory.\n";
    void* block = alloc.allocate(BLOCKSIZE);

    const char* str = "This memory will be shared!";
    memcpy(block, str, strlen(str));
    std::cout << reinterpret_cast<char*>(block) << "\n";

    int* array = reinterpret_cast<int*>(alloc.allocate(32));
    std::cout << "Allocating a 32B block of memory for an array of ints.\n";

    array[0] = 80;
    array[1] = 61;
    array[2] = 78;
    array[3] = 80;

    for (int i = 0; i < 4; i++) {
        std::cout << "Array has value " << array[i] << " at index " << i << "\n";
    }

    std::cout << "Deallocating the initial block.\n";
    if (alloc.deallocate(block) != 0) std::cout << "Error deallocating.\n";

    return 0;
}