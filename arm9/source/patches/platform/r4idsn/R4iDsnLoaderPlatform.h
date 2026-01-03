#pragma once
#include "common.h"
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "R4iDsnReadSdPatchCode.h"
#include "R4iDsnSdReadSectorPatchCode.h"
#include "R4iDsnWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the r4idsn.com flashcard.
class R4iDsnLoaderPlatform : public IoRpgLoaderPlatform
{
private:
    enum
    {
        IORPG_CMD_SDIO_BYTE = 0xAB
    };

public:
    R4iDsnLoaderPlatform() : IoRpgLoaderPlatform(IORPG_CMD_SDIO_BYTE) { }

    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            auto* waitForStatePatchCode = patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new IoRpgSdWaitForStatePatchCode(patchHeap);
            });
            return new R4iDsnReadSdPatchCode(patchHeap,
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new IoRpgSendSdioCommandPatchCode(patchHeap);
                }),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new R4iDsnSdReadSectorPatchCode(patchHeap, waitForStatePatchCode);
                }),
                waitForStatePatchCode);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new R4iDsnWriteSdPatchCode(patchHeap,
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
        r4idsn_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R4, THUMB_R0);
        r4idsn_writeSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
    }
};
