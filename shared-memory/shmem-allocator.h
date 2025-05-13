#include <sys/shm.h>

class SharedAllocator {
private:
    key_t key;
public:
    SharedAllocator(const char* keygen);
    ~SharedAllocator();

    void* allocate();
    int deallocate();

    void* attach();
    int detach();
};