#include <sys/shm.h>
#include <climits>

class SharedAllocator {
private:
    struct block_t {
        size_t size;
        block_t* next;
        block_t* prev;
        bool free;
        char data[1];
    };

    key_t key;
    int id;
    size_t size;
    size_t alcd;
    block_t* alcd_blocks;
    block_t* free_blocks;

    block_t* findSlot(size_t size) {
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

    void splitBlock(block_t* block, size_t size);

public:
    SharedAllocator(const char* keygen, size_t size);
    ~SharedAllocator();

    void* allocate(size_t size);
    int deallocate(block_t* addr);
};