#include <memory>
#include <queue>
#include <unordered_set>

#ifndef DEFAULT_BLOCK_SIZE
#define DEFAULT_BLOCK_SIZE 256
#endif

#ifndef DEFAULT_NUM_BLOCKS
#define DEFAULT_NUM_BLOCKS 256
#endif

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
     * Constructor initializes MemoryPool object with user-specified size & block-size and allocates sufficient space for the entire pool
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
 * Allows creation of a memory pool of user-specified size and block-size. Allocates and deallocates blocks at user request. Uses a linked list to track the blocks that are free to allocate. Next pointer is kept within each free block for space. 
 */
class LLMemoryPool {
private:
    void* pool;
    void* first_free;
    int block_size;
    int total_blocks;
    int free_blocks;

public:
    /**
     * Memory pool constructor
     * 
     * Constructor intializes LLMemoryPool object with user-specified size & block-size and allocates sufficient space for the entire pool
     * @param blocks Integer specifying number of blocks in the pool
     * @param size Integer specifying size of a single block
     */
    LLMemoryPool(int blocks, int size);

    /**
     * Memory pool destructor
     * 
     * Destructor frees the space of the entire pool
     */
    ~LLMemoryPool();

    /**
     * Allocate a single block of memory from the pool
     * 
     * Selects the address at the front of the free list and allocates its block to the user
     * @return void* to the address of the start of the allocated block
     */
    void* allocate();

    /**
     * Deallocate a single block of memory from the pool
     * 
     * Deallocates the block starting at the address passed in by the user
     * @param block void* pointing to the block to release. Must be the START of a block within the valid arena and must have been previously allocated.
     * @return 0 on success, -1 on failure
     */
    int deallocate(void* block);
};

template<typename T>
class MemoryPoolAllocator {
private:
    std::shared_ptr<LLMemoryPool> pool;
public:
    using value_type = T;

    MemoryPoolAllocator() {
        pool = std::make_shared<LLMemoryPool>(DEFAULT_NUM_BLOCKS, DEFAULT_BLOCK_SIZE);
    }

    MemoryPoolAllocator(size_t blocks, size_t block_size) {
        pool = std::make_shared<LLMemoryPool>(blocks, block_size);
    }

    template<typename U>
    constexpr MemoryPoolAllocator(const MemoryPoolAllocator<U> &rhs) {pool = rhs.pool;}

    T* allocate(size_t n) {
        return reinterpret_cast<T*>(pool->allocate());
    }

    void deallocate(T* block, size_t n) noexcept {
        // Don't have to worry about block being non-allocated since only called by STL
        pool->deallocate(reinterpret_cast<void*>(block));
    }

    friend bool operator==(const MemoryPoolAllocator &lhs, const MemoryPoolAllocator &rhs) {return lhs.pool == rhs.pool;}
    friend bool operator!=(const MemoryPoolAllocator &lhs, const MemoryPoolAllocator &rhs) {return lhs.pool != rhs.pool;}
};