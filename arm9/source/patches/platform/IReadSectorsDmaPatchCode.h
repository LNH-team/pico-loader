#pragma once

/// @brief Interface for patch code implementing dma sector reads.
class IReadSectorsDmaPatchCode
{
protected:
    IReadSectorsDmaPatchCode() { }

public:
    /// @brief Pointer to a function for reading a single SD sector using the given dmaChannel.
    ///        In first sector of batch read, previousSrcSector will be 0.
    ///        On further reads previousSrcSector will contains the previous srcSector
    ///        which can be used to continue a sequential read.
    typedef void (*ReadSectorsDmaFunc)(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);

    /// @brief Pointer to a function for finishing a batch of dma reads.
    typedef void (*ReadSectorsDmaFinishFunc)(void);

    /// @brief Gets a pointer to the dma SD read function in the patch code.
    /// @return The pointer to the dma SD read function.
    virtual const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const = 0;

    /// @brief Gets a pointer to the dma SD read finish function in the patch code.
    /// @return The pointer to the dma SD read finish function.
    virtual const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const = 0;
};
