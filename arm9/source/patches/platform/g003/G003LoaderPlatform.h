#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "g003ReadSdAsm.h"
#include "g003ReadSdDmaAsm.h"
#include "g003WriteSdAsm.h"

/// @brief Implementation of LoaderPlatform for the GMP-Z003 flashcard
class G003LoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new G003ReadSdPatchCode(patchHeap);
        });
    }

    const SdReadDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<G003ReadSdDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr);
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new G003WriteSdPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool HasDmaSdReads() const override { return true; }
};
