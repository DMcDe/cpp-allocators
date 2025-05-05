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
    // Does NOT error check that the address was previously allocated -- assumes user passes in previously allocated.
    // If user does not, could cause undefined behavior down the line by creating a loop in the free list.

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

LLMemoryPool::LLMemoryPool(int blocks, int size) : total_blocks(blocks), free_blocks(blocks), block_size(size) {
    // Get memory
    pool = malloc(total_blocks * block_size);
    if (!pool) throw std::bad_alloc();

    // Start of the pool is free at initialization
    first_free = pool;

    // Put the address of the next free block at the start of each block
    for (int i = 0; i < total_blocks - 1; i++) {
        *(void**)((char*)first_free + i * block_size) = (char*)first_free + (i + 1) * block_size;
    }

    // Put nullptr in the final block to mark end of list
    *(void**)((char*)first_free + (total_blocks - 1) * block_size) = nullptr;
}

void* LLMemoryPool::allocate() {
    // Only allow allocation if space exists
    if (free_blocks == 0) throw std::bad_alloc();

    // Get next available block
    void* block = first_free;

    // Remove from free list by setting first_free to point to the next block
    first_free = *((void**)first_free);

    // Decrement free blocks count
    free_blocks--;

    // Return address
    return block;
}

int LLMemoryPool::deallocate(void* block) {
    // Error check -- shouldn't be nullptr
    if (block == nullptr) return -1;

    // Deallocation address must be the start of a block
    if (((char*)block - (char*)pool) % block_size != 0) return -1;

    // Ensure address is within the arena
    if ((char*)block < (char*)pool || (char*)block > (char*)pool + total_blocks * block_size) return -1;

    // Set to point to what was previously front of free list
    *((void**)block) = first_free;

    // Append to front of free list
    first_free = block;

    // Increment free blocks count
    free_blocks++;

    return 0;
}

LLMemoryPool::~LLMemoryPool() {
    free(pool);
}