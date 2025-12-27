#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "DATELSpiCommandsAsm.h"
#include "DATELReadSectorsAsm.h"
#include "DATELWriteSectorsAsm.h"

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
            return new DATELSendSDIOCommandPatchCode(patchHeap, spi);
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
            return new DATELSendSDIOCommandPatchCode(patchHeap, spi);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DatelWriteSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    bool InitializeSdCard() override;
};
