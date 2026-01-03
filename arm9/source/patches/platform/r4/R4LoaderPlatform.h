#pragma once
#include "../LoaderPlatform.h"
#include "R4ReadRomPatchCode.h"
#include "R4ReadSdPatchCode.h"
#include "R4WriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the original R4 flashcard
class R4LoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new R4ReadSdPatchCode(patchHeap);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new R4WriteSdPatchCode(patchHeap);
        });
    }

    const IReadSectorsPatchCode* CreateRomReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new R4ReadRomPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool HasRomReads() const override { return true; }

    void PrepareRomBoot(u32 romDirSector, u32 romDirSectorOffset) const override;

private:
    u32 ReadCardInfo() const;
    void PrepareClusterMap(bool isSave, u32 dirSector, u32 dirSectorOffset) const;
};
