#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "ARDSSpiCommandsAsm.h"
#include "ARDSReadSectorsAsm.h"
#include "ARDSWriteSectorsAsm.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class ARDSLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSReadSpiBytePatchCode(patchHeap);
                });
        auto cycle = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSCycleSpiPatchCode(patchHeap, spi);
                });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSSendSDIOCommandPatchCode(patchHeap, spi, cycle);
                });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new ARDSReadSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto spi = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSReadSpiBytePatchCode(patchHeap);
                });
        auto cycle = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSCycleSpiPatchCode(patchHeap, spi);
                });
        auto sendSdio = patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new ARDSSendSDIOCommandPatchCode(patchHeap, spi, cycle);
                });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new ARDSWriteSdPatchCode(patchHeap, spi, sendSdio);
        });
    }

    bool InitializeSdCard() override;
};
