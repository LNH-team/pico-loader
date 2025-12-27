#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "ezpReadSectorsAsm.h"
#include "ezpReadSdDataAsm.h"
#include "ezpWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the EZ-Flash Parallel flashcard
class EZPLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new EZPReadSectorsPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new EZPReadSDDataPatchCode(patchHeap);
                }));
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new EZPWriteSectorsPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
