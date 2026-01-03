#pragma once
#include <string.h>
#include "PatchHeap.h"

/// @brief Class representing a block of patch code to be inserted in a \see PatchHeap.
class PatchCode
{
public:
    /// @brief Constructs a PatchCode instance for the given block of code
    ///        to be inserted in the given patchHeap.
    /// @param code The start of the block of code.
    /// @param size The size of the block of code.
    /// @param patchHeap The patch heap to use.
    PatchCode(const void* code, u32 size, PatchHeap& patchHeap)
        : _code(code), _size(size), _targetAddress(patchHeap.Alloc(size)) { }

    ~PatchCode()
    {
        LOG_FATAL("Patch code must not be deleted.\n");
        while (1);
    }

    /// @brief Copies the patch code to the target address.
    void CopyToTarget() const
    {
        memcpy(_targetAddress, _code, _size);
    }

protected:
    /// @brief The start of the code block.
    const void* const _code;

    /// @brief The size of the code block.
    const u32 _size;

    /// @brief The target address for the code block in the patch heap.
    void* const _targetAddress;

    /// @brief Converts a pointer inside the original code block
    ///        to a pointer at the target location in the patch heap.
    /// @param ptr The pointer to convert.
    /// @return The converted pointer.
    const void* GetAddressAtTarget(const void* ptr) const
    {
        return (const void*)((u32)ptr - (u32)_code + (u32)_targetAddress);
    }
};
