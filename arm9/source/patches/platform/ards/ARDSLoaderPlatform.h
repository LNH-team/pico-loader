#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "ARDSSpiCommandsAsm.h"
#include "ARDSReadSectorsAsm.h"
#include "ARDSWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class ARDSLoaderPlatform : public LoaderPlatform
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

    bool InitializeSdCard() override;
};
