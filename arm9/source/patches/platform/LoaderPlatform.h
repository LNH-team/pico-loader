#pragma once
#include "IReadSectorsPatchCode.h"
#include "IReadSectorsDmaPatchCode.h"
#include "IWriteSectorsPatchCode.h"
#include "../PatchHeap.h"
#include "../PatchCodeCollection.h"
#include "LoaderPlatformType.h"

/// @brief Abstract class for platform (flashcard or other sd access method) specific parts of the loader.
class LoaderPlatform
{
public:
    /// @brief Creates the SD read patch code for the platform.
    /// @param patchCodeCollection The patch code collection.
    /// @param patchHeap The patch heap.
    /// @return A pointer to the created SD read patch code.
    virtual const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const = 0;

    /// @brief Creates the SD read dma patch code for the platform.
    /// @param patchCodeCollection The patch code collection.
    /// @param patchHeap The patch heap.
    /// @return A pointer to the created SD read dma patch code.
    virtual const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const { return nullptr; }

    /// @brief Creates the SD write patch code for the platform.
    /// @param patchCodeCollection The patch code collection.
    /// @param patchHeap The patch heap.
    /// @return A pointer to the created SD write patch code.
    virtual const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const = 0;

    /// @brief Creates the rom read patch code for the platform.
    /// @param patchCodeCollection The patch code collection.
    /// @param patchHeap The patch heap.
    /// @return A pointer to the created rom read patch code.
    virtual const IReadSectorsPatchCode* CreateRomReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const { return nullptr; }

    /// @brief Returns the type of this loader platform.
    /// @return The type of this loader platform.
    virtual LoaderPlatformType GetPlatformType() const = 0;

    /// @brief Checks if the platform supports rom reads directly.
    /// @return True if the platform supports rom reads, or false otherwise.
    virtual bool HasRomReads() const { return false; }

    /// @brief Checks if the platform supports sd reads via dma.
    /// @return True if the platform supports sd reads via dma, or false otherwise.
    virtual bool HasDmaSdReads() const { return false; }

    /// @brief Prepares the platform for running a rom.
    /// @param romDirSector The directory sector of the rom file, or 0 if not applicable.
    /// @param romDirSectorOffset The byte offset of the rom fat entry in the directory sector.
    virtual void PrepareRomBoot(u32 romDirSector, u32 romDirSectorOffset) const { }

    /// @brief For platforms that need it this function may be used to (re)initialize the sd card.
    /// @return True if the sd card initialization was successful, or false otherwise.
    virtual bool InitializeSdCard() { return true; }
};
