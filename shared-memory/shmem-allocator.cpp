#include "shmem-allocator.h"
#include <cerrno>
#include <climits>
#include <new>

SharedAllocator::SharedAllocator(const char* keygen, size_t size) {
    // Generate a key from the given generator string
    key = ftok(keygen, 'a');
    if (key == -1) throw std::bad_alloc();

    // Create memory segment if it doesn't exist
    bool initialized = false;
    id = shmget(key, size, 0644 | IPC_CREAT | IPC_EXCL);

    // If memory segment already exists, just join onto it
    if (id == -1 && errno == EEXIST) {
        id = shmget(key, size, 0644 | IPC_CREAT);
        if (id == -1) throw std::bad_alloc();
        initialized = true;
    }  

    // Attach
    arena = shmat(id, (void*)0, 0);
    if (arena == (void*)-1) throw std::bad_alloc();

    header = reinterpret_cast<header_t*>(arena);

    // If not the ones creating the segment, don't need to intialize it
    if (initialized) return; 

    // Initialize memory segment
    header->free_blocks = reinterpret_cast<block_t*>(arena);
    header->free_blocks->size = size;
    header->free_blocks->next = nullptr;
    header->free_blocks->prev = nullptr;
    header->free_blocks->free = true;

    // No memory has been allocated yet
    header->alcd_blocks = nullptr;
    header->alcd = 0;
    header->size = size;
}

SharedAllocator::~SharedAllocator() {
    // This should never fail, but throw if it does
    if (shmdt(arena) == -1) throw std::bad_alloc();

    // Remove the segment
    if (shmctl(id, IPC_RMID, 0) == -1) throw std::bad_alloc();
    // TODO: Validate this always works -- might need to change permissions (0644) in the shmget command for it to
}

void* SharedAllocator::allocate(size_t size) {
    if (size == 0) return nullptr;

    // Pop block that best fits
    block_t* block = findSlot(size);

    // Fail to allocate if lack sufficient space
    if (block == nullptr) throw std::bad_alloc(); // TODO: Could resize here, but would require creating whole new memory region & copying over

    // Split the block so as to only use the amount of space we need
    splitBlock(block, size);

    // Remove from free list
    if (block->prev) block->prev->next = block->next;
    else header->free_blocks = block->next; // If no prev, it was at the start of the free list
    
    block->free = false;

    if (block->next) block->next->prev = block->prev;

    // Update allocated
    header->alcd += size;
    block->next = header->alcd_blocks;
    block->prev = nullptr;
    header->alcd_blocks = block;

    // Return a pointer to its data
    return reinterpret_cast<void*>(block->data);
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

int SharedAllocator::deallocate(void* addr) {
    if (!addr) return -1;

    // Get struct address from memory address
    block_t* block = reinterpret_cast<block_t*>(reinterpret_cast<char*>(addr) - sizeof(block_t));

    // Update free & size
    block->free = true;
    header->alcd_blocks -= block->size;

    // Remove from allocated list
    if (block->prev) block->prev->next = block->next;
    else header->alcd_blocks = block->next; // In this case, was at front of allocated list

    // Update pointers
    if (block->next) block->next->prev = block->prev;

    // Add to free list
    block->next = header->free_blocks;
    block->prev = nullptr;

    if (header->free_blocks) header->free_blocks->prev = block;
    header->free_blocks = block;

    // Recombine adjacent blocks if possible to reduce fragmentation
    combineBlocks(block);

    return 0;
}

SharedAllocator::block_t* SharedAllocator::findSlot(size_t size) {
    block_t* blk = header->free_blocks;
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

void SharedAllocator::combineBlocks(block_t* block) {
    // Merge with next block, if free
    if (block->next && block->next->free) {
        // Combine sizes
        block->size += block->next->size;

        // Update relevant pointers
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    // Merge into previous block, if free
    if (block->prev && block->prev->free) {
        // Combine sizes
        block->prev->size += block->size;

        // Update relevant pointers
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;

        // Block is no longer start of a block -- update it
        block = block->prev;
    }
}