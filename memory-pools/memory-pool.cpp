#include "memory-pool.h"
#include <cstdlib>
#include <cstdint>

MemoryPool::MemoryPool(int blocks, int size) : total_blocks(blocks), block_size(size) {
    // Get memory
    pool = malloc(total_blocks * block_size);
    if (!pool) throw std::bad_alloc();

    // Initialize list of free blocks
    for (int i = 0; i < blocks; i++) free_blocks.push(i);
}

void* MemoryPool::allocate() {
    // Only allow allocation if space exists
    if (free_blocks.empty()) throw std::bad_alloc();

    // Pop a block from the list of available
    int block_num = free_blocks.front();
    free_blocks.pop();
    used_blocks.insert(block_num);

    // Get address of that block
    uint32_t offset = block_size * block_num;
    char* block = reinterpret_cast<char*>(pool) + offset;

    // Return the address of the block
    return reinterpret_cast<void*>(block);
}

int MemoryPool::deallocate(void* block) {
    // Error check -- shouldn't be nullptr
    if (!block) return -1;

    // Find the block number corresponding to that address
    char* ptr = reinterpret_cast<char*>(block);

    // Deallocation address must be the start of a block
    if ((ptr - reinterpret_cast<char*>(pool)) % block_size != 0) return -1;
    int block_num = (ptr - reinterpret_cast<char*>(pool)) / block_size;

    // Ensure address is within the bounds of the pool
    if (block_num < 0 || block_num > total_blocks) return -1;

    // Ensure block was previously allocated
    if (used_blocks.erase(block_num) == 0) return -1;

    // Return the block to list of usable blocks
    free_blocks.push(block_num);

    return 0;
}

MemoryPool::~MemoryPool() {
    free(pool);
}