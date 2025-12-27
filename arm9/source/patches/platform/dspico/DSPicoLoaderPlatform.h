#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "dspicoReadSectorsAsm.h"
#include "dspicoReadSdSectorDmaAsm.h"
#include "dspicoWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the DS pico flashcard
class DSPicoLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DSPicoReadSdSectorsPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DSPicoReadSdSectorsDirectPatchCode(patchHeap);
                }));
        });
    }

    const SdReadDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        auto pollSdDataReadyPatchCode = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DSPicoReadSdSectorDmaPollSdDataReadyPatchCode(patchHeap);
        });

        return patchCodeCollection.AddUniquePatchCode<DSPicoReadSdSectorDmaPatchCode>(
            patchHeap, pollSdDataReadyPatchCode, miiCardDmaCopy32Ptr);
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DSPicoWriteSdSectorsPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool HasDmaSdReads() const override { return true; }
};
