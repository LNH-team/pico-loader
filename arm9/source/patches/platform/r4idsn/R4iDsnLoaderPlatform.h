#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "../acekard-common/IoRpgDefinitions.h"
#include "R4iDsnReadSdPatchCode.h"
#include "R4iDsnSdReadSectorPatchCode.h"
#include "R4iDsnWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the Acekard 2 flashcard
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
            return new R4iDsnReadSdPatchCode(patchHeap,
                CreateSdHelperPatchCode(patchCodeCollection, patchHeap),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new R4iDsnSdReadSectorPatchCode(
                        patchHeap,
                        CreateSdHelperPatchCode(patchCodeCollection, patchHeap)
                    );
                }));
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new R4iDsnWriteSdPatchCode(patchHeap,
                CreateSdHelperPatchCode(patchCodeCollection, patchHeap)
            );
        });
    }

    void PatchSdscShift(void) const override
    {
        r4idsn_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R4, THUMB_R0);
        r4idsn_writeSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
    }

    const IoRpgPlatformSpecifics& GetPlatformSpecifics(void) const override
    {
        static const IoRpgPlatformSpecifics data
        {
            .cmd12Command = 0x0C0001AB,
            .cmd17Command = 0x110003AB,
            .cmd18Command = 0x120004AB,
            .cmd24Command = 0x180005AB,
            .sdStateShift = 4
        };
        return data;
    }
};
