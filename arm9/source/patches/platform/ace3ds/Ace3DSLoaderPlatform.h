#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "ace3dsReadSdAsm.h"
#include "ace3dsReadSdDmaAsm.h"
#include "ace3dsWriteSdAsm.h"

/// @brief Implementation of LoaderPlatform for the Ace3DS+ flashcard
class Ace3DSLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ace3DSReadSdPatchCode(patchHeap);
        });
    }

    const SdReadDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<Ace3DSReadSdDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr);
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
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
