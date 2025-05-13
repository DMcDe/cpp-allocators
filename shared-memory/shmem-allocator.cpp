#include "shmem-allocator.h"
#include <climits>
#include <new>

SharedAllocator::SharedAllocator(const char* keygen, size_t size) : size(size) {
    // Generate a key from the given generator string
    key = ftok(keygen, 'a');
    if (key == -1) throw std::bad_alloc();

    // Create memory segment if it doesn't exist (connect to it if it does)
    id = shmget(key, size, 0644 | IPC_CREAT);
    if (id == -1) throw std::bad_alloc();

    void* allocation = shmat(id, (void*)0, 0);
    if (allocation == (void*)-1) throw std::bad_alloc();

    // Initialize free space struct
    free_blocks = reinterpret_cast<block_t*>(allocation);
    free_blocks->size = size;
    free_blocks->next = nullptr;
    free_blocks->prev = nullptr;
    free_blocks->free = true;

    // No memory has been allocated yet
    alcd_blocks = nullptr;
    alcd = 0;
}

SharedAllocator::~SharedAllocator() {
    // This should never fail, but throw if it does
    if (shmdt(reinterpret_cast<void*>(free_blocks)) == -1) throw std::bad_alloc();

    // Remove the segment
    if (shmctl(id, IPC_RMID, 0) == -1) throw std::bad_alloc();
    // TODO: Validate this always works -- might need to change permissions (0644) in the shmget command for it to
}

void* SharedAllocator::allocate(size_t size) {
    // Pop block that best fits

    // Assign its size?

    // Update prev with new size

    // Update free

    // Update pointers

    // Return a pointer to its data
}

void SharedAllocator::splitBlock(block_t* block, size_t size) {
    // Return if can't make another block out of remaining space
    if (block->size - size < sizeof(block_t)) return;

    // Set next block to begin after [size] bytes & update ptrs
    block_t* new_block = reinterpret_cast<block_t*>(reinterpret_cast<char*>(block) + size);
    new_block->prev = block;
    new_block->next = block->next;
    if (block->next) (block->next)->prev = new_block;

    block->next = new_block;

    // New block is unused
    new_block->free = true;

    // Update sizes
    size_t new_size = block->size - size;
    new_block->size = new_size;
    block->size = size;
}

SharedAllocator::block_t* SharedAllocator::findSlot(size_t size) {
    block_t* blk = free_blocks;
    block_t* best_blk = nullptr;
    size_t best_dif = INT_MAX;

    while (blk != nullptr) {
        if (blk->free && blk->size > size) {
            size_t dif = blk->size - size;
            if (dif < best_dif) {
                best_dif = dif;
                best_blk = blk;

                if (dif == 0) break;
            }
        }

        blk = blk->next;
    }

    return best_blk;
}