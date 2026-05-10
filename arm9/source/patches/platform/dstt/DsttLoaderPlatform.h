#pragma once
#include "../LoaderPlatform.h"
#include "DsttReadSdPatchCode.h"
#include "DsttReadSdDmaPatchCode.h"
#include "DsttWriteSdPatchCode.h"
#include "DsttSdStopTransmissionPatchCode.h"
#include "DsttReadSdHelperPatchCode.h"

/// @brief Implementation of LoaderPlatform for the DSTT flashcard
class DsttLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DsttReadSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttSdStopTransmissionPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttReadSdHelperPatchCode(patchHeap);
                }));
        });
    }

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        return patchCodeCollection.AddUniquePatchCode<DsttReadSdDmaPatchCode>(
            patchHeap, miiCardDmaCopy32Ptr,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttSdStopTransmissionPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttReadSdHelperPatchCode(patchHeap);
                }));
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DsttWriteSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttWriteSdContinuePatchCode(patchHeap);
                }));
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool HasDmaSdReads() const override { return true; }

    bool InitializeSdCard() override;
};
