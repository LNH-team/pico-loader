#pragma once

/// @brief Interface for patch code implementing sector reads.
class IReadSectorsPatchCode
{
protected:
    IReadSectorsPatchCode() { }

public:
    /// @brief Pointer to a function for reading sectorCount sectors
    ///        from srcSector to the given dst buffer.
    typedef void (*ReadSectorsFunc)(u32 srcSector, void* dst, u32 sectorCount);

    /// @brief Gets a pointer to the sector read function in the patch code.
    /// @return The pointer to the sector read function.
    virtual const ReadSectorsFunc GetReadSectorsFunction() const = 0;
};
