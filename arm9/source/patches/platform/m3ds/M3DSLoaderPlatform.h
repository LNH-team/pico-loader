#pragma once
#include "../LoaderPlatform.h"
#include "M3DSReadSdSectorsDmaPatchCode.h"
#include "M3DSReadSdSectorsPatchCode.h"
#include "M3DSWriteSdSectorsPatchCode.h"

/// @brief Implementation of LoaderPlatform for the M3 DS Real flashcard
class M3DSLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
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

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<M3DSReadSdSectorsDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr,
            patchCodeCollection.AddUniquePatchCode<M3DSReadSdSectorsDmaHelperPatchCode>(patchHeap)
        );
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
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

    bool HasDmaSdReads() const override { return true; }
};
