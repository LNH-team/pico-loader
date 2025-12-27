#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "m3dsReadSectorsAsm.h"
#include "m3dsWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the M3 DS Real flashcard
class M3DSLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new M3DSReadSdSectorsPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new M3DSReceiveSectorPatchCode(patchHeap);
                }));
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new M3DSWriteSdSectorsPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new M3DSSendSectorPatchCode(patchHeap);
                }));
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }
};
