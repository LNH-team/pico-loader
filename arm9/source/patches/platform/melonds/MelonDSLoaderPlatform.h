#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "melondsReadSdAsm.h"
#include "melondsWriteSdAsm.h"

/// @brief Implementation of LoaderPlatform for MelonDS
class MelonDSLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new MelonDSReadSdPatchCode(patchHeap);
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new MelonDSWriteSdPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
