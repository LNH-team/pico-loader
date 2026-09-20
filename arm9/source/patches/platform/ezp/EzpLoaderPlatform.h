#pragma once
#include "../LoaderPlatform.h"
#include "EzpReadSectorsPatchCode.h"
#include "EzpReadSectorsDmaPatchCode.h"
#include "EzpReadSdDataPatchCode.h"
#include "EzpWriteSectorsPatchCode.h"

/// @brief Implementation of LoaderPlatform for the EZ-Flash Parallel flashcard
class EzpLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new EzpReadSectorsPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new EzpReadSdDataPatchCode(patchHeap);
                }));
        });
    }

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<EzpReadSectorsDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr);
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new EzpWriteSectorsPatchCode(patchHeap);
        });
    }

    bool HasDmaSdReads() const override { return true; }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
