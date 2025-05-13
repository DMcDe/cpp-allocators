#include <sys/shm.h>

class SharedAllocator {
private:
    key_t key;
    int id;
    size_t size;
    void* arena;

public:
    SharedAllocator(const char* keygen, size_t size);
    ~SharedAllocator();

    void* allocate();
    int deallocate();

    void* attach();
    int detach();
};