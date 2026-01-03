#pragma once
#include "../LoaderPlatform.h"
#include "EzpReadSectorsPatchCode.h"
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

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new EzpWriteSectorsPatchCode(patchHeap);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
