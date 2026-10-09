#pragma once
#include <array>

/// @brief Class implementing a heap intended for patch code.
///        The heap consists of a linked list of blocks of free
///        space. It is possible to add many small pieces.
class PatchHeap
{
public:
    PatchHeap();

    /// @brief Adds a block of free space to the patch heap.
    /// @param block The start of the block.
    /// @param size The size of the block.
    void AddFreeSpace(void* block, u32 size);

    /// @brief Allocated a block of the given size from the patch heap.
    /// @param size The size to allocate.
    /// @return A pointer to the allocated block if successful, or nullptr otherwise.
    void* Alloc(u32 size);

    /// @brief Optional allocation: returns nullptr without stopping on exhaustion.
    void* TryAlloc(u32 size);
    bool TryAllocPieces(const u32* sizes, void** outputs, u32 count);

    /// @brief Read-only capacity diagnostics; allocations require a contiguous block.
    u32 GetFreeBytes() const;
    u32 GetLargestFreeBlock() const;

private:
    struct PatchHeapBlock
    {
        PatchHeapBlock* next;
        void* block;
        u32 size;
    };

    std::array<PatchHeapBlock, 64> _blocks;
    PatchHeapBlock* _unusedBlockPool;

    PatchHeapBlock* _freeBlocks;
};
