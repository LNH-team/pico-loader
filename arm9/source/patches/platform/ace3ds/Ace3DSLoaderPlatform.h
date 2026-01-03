#pragma once
#include "../LoaderPlatform.h"
#include "Ace3DSReadSdPatchCode.h"
#include "Ace3DSReadSdDmaPatchCode.h"
#include "Ace3DSWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the Ace3DS+ flashcard
class Ace3DSLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ace3DSReadSdPatchCode(patchHeap);
        });
    }

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<Ace3DSReadSdDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr);
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ace3DSWriteSdPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool HasDmaSdReads() const override { return true; }
};
