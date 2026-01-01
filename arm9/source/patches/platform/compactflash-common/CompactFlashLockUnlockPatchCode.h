#pragma once
#include "common.h"
#include "../../PatchCode.h"

/// @brief Abstract base class for patch code implementing Compact Flash carts lock/unlock routines
class CompactFlashLockUnlockPatchCode : public PatchCode
{
protected:
    CompactFlashLockUnlockPatchCode(const void* code, u32 size, PatchHeap& patchHeap)
        : PatchCode(code, size, patchHeap) { }

public:
    /// @brief Gets a pointer to the SD write function in the patch code.
    /// @return The pointer to the SD write function.
    virtual const void* GetLockUnlockFunction() const = 0;
};
