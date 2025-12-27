#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "DATELSpiCommandsAsm.h"
#include "DATELReadSectorsAsm.h"
#include "DATELWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class DATELLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELReadSpiBytePatchCode(patchHeap);
                });
        auto cycle = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELCycleSpiPatchCode(patchHeap);
                });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELSendSDIOCommandPatchCode(patchHeap, spi, cycle);
                });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DATELReadSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELReadSpiBytePatchCode(patchHeap);
                });
        auto cycle = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELCycleSpiPatchCode(patchHeap);
                });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new DATELSendSDIOCommandPatchCode(patchHeap, spi, cycle);
                });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new DATELWriteSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    bool InitializeSdCard() override;
};
