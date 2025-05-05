#include <queue>
#include <unordered_set>

/**
 * Implementation of a memory pool-based allocator
 * 
 * Allows creation of a memory pool of user-specified size and block-size. Allocates and deallocates blocks at user request.
 */
class MemoryPool {
private:
    void* pool;
    std::queue<int> free_blocks;
    std::unordered_set<int> used_blocks;
    int total_blocks;
    int block_size;
public:
    /**
     * Memory pool constructor
     * 
     * Constructor initializes MemoryPool object with user-specific size & block-size and allocates sufficient space for the entire pool
     * @param blocks Integer specifying number of blocks in the pool
     * @param size Integer specifying the size of a single block
     */
    MemoryPool(int blocks, int size);

    /**
     * Memory pool destructor
     * 
     * Destructor free the space of the entire pool
     */
    ~MemoryPool();

    /**
     * Allocate a single block of memory from the pool
     * 
     * Pops the first block marked available, allocates it to the caller, and removes it from the list of available blocks
     * @return void* to the address of the start of the allocated block
     */
    void* allocate();

    /**
     * Deallocate a single block of memory from the pool
     * 
     * Deallocates the block starting at the address passed-in by the user
     * @param block void* to the beginning of a previously-allocated block. The address must point to the START of a block and must have been previously allocated in a call to allocate()
     * @return 0 on success, -1 on failure
     */
    int deallocate(void* block);
};

/**
 * Linked list-based Implementation of a memory-pool allocator
 * 
 * Allows creation of a memory pool of user-specified size and block-size. Allocates and deallocates blocks at user request. Uses a linked list to track the blocks that are free to allocate.
 */
class LLMemoryPool {
private:
    void* alloc_head;
    void* free_head;
    int block_size;
    int total_blocks;
    int free_blocks;

public:
    LLMemoryPool(int blocks, int size);
    ~LLMemoryPool();

    void* allocate();
    int deallocate(void* block);
};