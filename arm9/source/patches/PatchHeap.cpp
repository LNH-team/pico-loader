#include "common.h"
#include "PatchHeap.h"
#include "errorDisplay/ErrorDisplay.h"

u32 retailReturnPatchStage = 0;
u32 retailReturnFailedAllocation = 0;

PatchHeap::PatchHeap()
    : _freeBlocks(nullptr)
{
    _unusedBlockPool = &_blocks[0];
    for (u32 i = 0; i < _blocks.size() - 1; i++)
        _blocks[i].next = &_blocks[i + 1];
    _blocks[_blocks.size() - 1].next = nullptr;
}

void PatchHeap::AddFreeSpace(void* block, u32 size)
{
    PatchHeapBlock* cur = _freeBlocks;

    bool added = false;
    while (cur)
    {
        if ((u8*)cur->block + cur->size == block)
        {
            cur->size += size;
            added = true;
            break;
        }
        else if ((u8*)block + size == cur->block)
        {
            cur->block = block;
            cur->size += size;
            added = true;
            break;
        }

        cur = cur->next;
    }

    if (added)
        return;

    PatchHeapBlock* heapBlock = _unusedBlockPool;
    _unusedBlockPool = _unusedBlockPool->next;

    heapBlock->block = block;
    heapBlock->size = size;
    heapBlock->next = _freeBlocks;
    _freeBlocks = heapBlock;
}

u32 PatchHeap::GetFreeBytes() const
{
    u32 total = 0;
    for (auto block = _freeBlocks; block; block = block->next)
        total += block->size;
    return total;
}

u32 PatchHeap::GetLargestFreeBlock() const
{
    u32 largest = 0;
    for (auto block = _freeBlocks; block; block = block->next)
        if (block->size > largest)
            largest = block->size;
    return largest;
}

void* PatchHeap::Alloc(u32 size)
{
    void* result = TryAlloc(size);
    if (!result)
    {
        retailReturnFailedAllocation = size;
        // Show actual capacity at the required-allocation failure, before spinning.
        ErrorDisplay().PrintPatchSpaceDiagnostic(0, 0, GetFreeBytes(), GetLargestFreeBlock());
        while (1);
    }
    return result;
}

void* PatchHeap::TryAlloc(u32 size)
{
    PatchHeapBlock* prev = nullptr;
    PatchHeapBlock* cur = _freeBlocks;

    PatchHeapBlock* bestBlock = nullptr;
    PatchHeapBlock* bestBlockPrev = nullptr;

    while (cur)
    {
        if (cur->size >= size && (!bestBlock || cur->size < bestBlock->size))
        {
            bestBlock = cur;
            bestBlockPrev = prev;
            if (bestBlock->size == size)
                break;
        }

        prev = cur;
        cur = cur->next;
    }

    if (!bestBlock)
        return nullptr;

    void* result = bestBlock->block;

    if (bestBlock->size == size)
    {
        if (bestBlockPrev)
            bestBlockPrev->next = bestBlock->next;

        if (_freeBlocks == bestBlock)
            _freeBlocks = bestBlock->next;

        bestBlock->next = _unusedBlockPool;
        _unusedBlockPool = bestBlock;
    }
    else
    {
        bestBlock->block = (u8*)bestBlock->block + size;
        bestBlock->size -= size;
    }

    LOG_DEBUG("Allocated 0x%p, size = 0x%X\n", result, size);

    return result;
}


bool PatchHeap::TryAllocPieces(const u32* sizes, void** outputs, u32 count)
{
    const auto blocks = _blocks;
    auto freeBlocks = _freeBlocks;
    auto unusedBlocks = _unusedBlockPool;
    for (u32 i = 0; i < count; ++i)
    {
        auto raw = (u8*)TryAlloc((sizes[i] + 7) & ~3u);
        if (!raw)
        {
            _blocks = blocks;
            _freeBlocks = freeBlocks;
            _unusedBlockPool = unusedBlocks;
            for (u32 j = 0; j < count; ++j) outputs[j] = nullptr;
            return false;
        }
        outputs[i] = (void*)(((u32)raw + 3) & ~3u);
    }
    return true;
}
