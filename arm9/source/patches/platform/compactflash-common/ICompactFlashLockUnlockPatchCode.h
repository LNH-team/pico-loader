#pragma once

/// @brief Interface for patch code implementing Compact Flash carts lock/unlock routines
class ICompactFlashLockUnlockPatchCode
{
protected:
    ICompactFlashLockUnlockPatchCode() { }

public:
    /// @brief Gets a pointer to the SD write function in the patch code.
    /// @return The pointer to the SD write function.
    virtual const void* GetLockUnlockFunction() const = 0;
};
