#pragma once

/// @brief Interface for patch code implementing sector writes.
class IWriteSectorsPatchCode
{
protected:
    IWriteSectorsPatchCode() { }

public:
    /// @brief Pointer to a function for writing sectorCount sectors
    ///        from the given src buffer to dstSector.
    typedef void (*WriteSectorsFunc)(u32 dstSector, const void* src, u32 sectorCount);

    /// @brief Gets a pointer to the sector write function in the patch code.
    /// @return The pointer to the sector write function.
    virtual const WriteSectorsFunc GetWriteSectorFunction() const = 0;
};
