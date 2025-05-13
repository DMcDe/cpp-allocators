#include "shmem-allocator.h"
#include <new>

SharedAllocator::SharedAllocator(const char* keygen, size_t size) {
    key = ftok(keygen, 'a');
    if (key == -1) throw std::bad_alloc();

    int id = shmget(key, size, 0644 | IPC_CREAT);
    if (id == -1) throw std::bad_alloc();

    arena = shmat(id, (void*)0, 0);
    if (arena == (void*)-1) throw std::bad_alloc();
}