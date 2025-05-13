#include "shmem-allocator.h"
#include <new>

SharedAllocator::SharedAllocator(const char* keygen, size_t size) : size(size) {
    // Generate a key from the given generator string
    key = ftok(keygen, 'a');
    if (key == -1) throw std::bad_alloc();

    // Create memory segment if it doesn't exist (connect to it if it does)
    id = shmget(key, size, 0644 | IPC_CREAT);
    if (id == -1) throw std::bad_alloc();

    arena = shmat(id, (void*)0, 0);
    if (arena == (void*)-1) throw std::bad_alloc();
}

SharedAllocator::~SharedAllocator() {
    // This should never fail, but throw if it does
    if (shmdt(arena) == -1) throw std::bad_alloc();

    // Remove the segment
    if (shmctl(id, IPC_RMID, 0) == -1) throw std::bad_alloc();
    // TODO: Validate this always works -- might need to change permissions (0644) in the shmget command for it to
}