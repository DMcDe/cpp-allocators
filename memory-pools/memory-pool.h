#include <queue>

class MemoryPool {
private:
    void* pool;
    std::queue<int> free_blocks;
    int total_blocks;
    int block_size;
public:
    MemoryPool(int blocks, int size);
    ~MemoryPool();

    void* allocate();
    int deallocate(void* block);
};