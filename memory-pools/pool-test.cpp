#include "memory-pool.h"
#include <cstring>
#include <iostream>

#ifndef NUM_BLOCKS
#define NUM_BLOCKS 256
#endif

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

int main() {
    MemoryPoolAllocator<int> alloc(NUM_BLOCKS, BLOCK_SIZE);
    
    std::vector<int, MemoryPoolAllocator<int>> vec(alloc);

    vec.push_back(3);
    vec.push_back(1);
    vec.push_back(2);

    for (int i : vec) std::cout << i << " ";
    std::cout << "\n";

    return 0;
}

// ---------------------------------------------------------------------------
// Test of non-STL integrated version
/*
int main() {
    LLMemoryPool pool(NUM_BLOCKS, BLOCK_SIZE);

    std::cout << "Allocating a block of memory.\n";
    void* block = pool.allocate();

    const char* str = "This is my first block of memory!\0";

    memcpy(block, str, strlen(str));
    std::cout << reinterpret_cast<char*>(block) << "\n";

    void* newblocks[5];
    for (int i = 0; i < 5; i++) {
        newblocks[i] = pool.allocate();
        std::cout << "Allocating a new block of memory.\n";
    }

    std::cout << "Freeing the first block.\n";
    if (pool.deallocate(block) != 0) {
        std::cout << "Error in deallocation.\n";
    }
    
    return 0;
}

*/