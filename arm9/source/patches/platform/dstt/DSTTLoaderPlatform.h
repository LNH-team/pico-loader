#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "dsttReadSdAsm.h"
#include "dsttWriteSdAsm.h"

/// @brief Implementation of LoaderPlatform for the DSTT flashcard
class DSTTLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DSTTReadSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DSTTReadSdStopTransmissionPatchCode(patchHeap);
                }));
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DSTTWriteSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DSTTWriteSdContinuePatchCode(patchHeap);
                }));
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;
};
