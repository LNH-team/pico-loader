#pragma once
#include "../LoaderPlatform.h"
#include "Ez5hReadSdSectorPatchCode.h"
#include "Ez5hReadSdDmaPatchCode.h"
#include "Ez5hDoSdOperationPatchCode.h"
#include "Ez5hSendCommandPatchCode.h"
#include "Ez5hWriteSdSectorPatchCode.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class Ez5hLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto sendCommand = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSendCommandPatchCode(patchHeap);
        });
        auto doSdOperation = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hDoSdOperationPatchCode(patchHeap, sendCommand);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hReadMultipleSdSectorPatchCode(patchHeap, doSdOperation);
        });
    }

    const IReadSectorsDmaPatchCode* CreateSdReadDmaPatchCode(PatchCodeCollection& patchCodeCollection,
        PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr) const override
    {
        auto sendCommand = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSendCommandPatchCode(patchHeap);
        });
        return patchCodeCollection.AddUniquePatchCode<Ez5hReadSdDmaPatchCode>(
            patchHeap, sendCommand, miiCardDmaCopy32Ptr);
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto sendCommand = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSendCommandPatchCode(patchHeap);
        });
        auto doSdOperation = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hDoSdOperationPatchCode(patchHeap, sendCommand);
        });
        auto writeSdSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hWriteSdSectorPatchCode(patchHeap);
        });
        auto sdioCrc = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSdioCrcPatchCode(patchHeap);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hWriteMultipleSectorPatchCode(patchHeap, writeSdSector, doSdOperation, sendCommand, sdioCrc);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;

    bool HasDmaSdReads() const override { return true; }
};
