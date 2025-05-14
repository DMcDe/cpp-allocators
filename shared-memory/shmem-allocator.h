#include <semaphore.h>
#include <sys/shm.h>

class SharedAllocator {
private:
    struct block_t {
        size_t size;
        block_t* next;
        block_t* prev;
        bool free;
        char data[1];
    };

    struct header_t {
        size_t size;
        size_t alcd;
        block_t* alcd_blocks;
        block_t* free_blocks;
    };
    
    const char* sem_name;
    key_t key;
    int id;
    void* arena;
    sem_t* mutex;
    header_t* header;

    block_t* findSlot(size_t size);
    void splitBlock(block_t* block, size_t size);
    void combineBlocks(block_t* block);

public:
    SharedAllocator(const char* keygen, size_t size);
    ~SharedAllocator();

    void* allocate(size_t size);
    int deallocate(void* addr);
};