#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "Ak2ReadSdPatchCode.h"
#include "Ak2SdReadSectorPatchCode.h"
#include "Ak2WriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the Acekard 2 flashcard
class Ak2LoaderPlatform : public IoRpgLoaderPlatform
{
private:
    enum
    {
        IORPG_CMD_SDIO_BYTE = 0xD5
    };

public:
    Ak2LoaderPlatform() : IoRpgLoaderPlatform(IORPG_CMD_SDIO_BYTE) { }

    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            auto* waitForStatePatchCode = patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new IoRpgSdWaitForStatePatchCode(patchHeap);
            });
            return new Ak2ReadSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new IoRpgSendSdioCommandPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new Ak2SdReadSectorPatchCode(patchHeap, waitForStatePatchCode);
                }),
                waitForStatePatchCode);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ak2WriteSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new IoRpgSendSdioCommandPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new IoRpgSdWaitForStatePatchCode(patchHeap);
                }));
        });
    }

    void PatchSdscShift(void) const override
    {
        ak2_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R4, THUMB_R0);
        ak2_writeSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
    }
};
