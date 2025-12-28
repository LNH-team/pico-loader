#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "DatelSpiCommandsAsm.h"
#include "DatelReadSectorsAsm.h"
#include "DatelWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class DatelLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSpiBytePatchCode(patchHeap);
        });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelSendSDIOCommandPatchCode(patchHeap, spi);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelReadSpiBytePatchCode(patchHeap);
        });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelSendSDIOCommandPatchCode(patchHeap, spi);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelWriteSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;
};
