#pragma once

/// @brief Interface for sector remapping patch code.
class ISectorRemapPatchCode
{
protected:
    ISectorRemapPatchCode() { }

public:
    /// @brief Gets a pointer to the sector remap function in the patch code.
    /// @return The pointer to the sector remap function.
    virtual const void* GetRemapFunction() const = 0;
};
