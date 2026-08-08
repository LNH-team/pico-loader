#pragma once
#include "../LoaderPlatform.h"
#include "MelonDSReadSdPatchCode.h"
#include "MelonDSReadSdDmaPatchCode.h"
#include "MelonDSWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for MelonDS
class MelonDSLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new MelonDSReadSdPatchCode(patchHeap);
        });
    }

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap,
        const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new MelonDSReadSdDmaPatchCode(patchHeap, miiCardDmaCopy32Ptr);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new MelonDSWriteSdPatchCode(patchHeap);
        });
    }

    bool HasDmaSdReads() const override { return true; }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
