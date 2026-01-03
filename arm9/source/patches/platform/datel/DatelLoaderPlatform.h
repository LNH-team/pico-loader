#pragma once
#include "../LoaderPlatform.h"
#include "DatelSendSdioCommandPatchCode.h"
#include "DatelReadSdPatchCode.h"
#include "DatelWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class DatelLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSpiBytePatchCode(patchHeap);
        });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelSendSdioCommandPatchCode(patchHeap, spi);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSpiBytePatchCode(patchHeap);
        });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelSendSdioCommandPatchCode(patchHeap, spi);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelWriteSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;
};
