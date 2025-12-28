#pragma once
#include "../LoaderPlatform.h"
#include "DsttReadSdPatchCode.h"
#include "DsttWriteSdPatchCode.h"
#include "DsttSdStopTransmissionPatchCode.h"
#include "DsttReadSdApplySectorCommandPatchCode.h"
#include "DsttReadSdWaitDataReadyPatchCode.h"
#include "DsttReadSdTransferDataPatchCode.h"

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
                    return new DsttReadSdApplySectorCommandPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttReadSdWaitDataReadyPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DsttReadSdTransferDataPatchCode(patchHeap);
                }));
        });
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

    bool InitializeSdCard() override;
};
