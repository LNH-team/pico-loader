#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "../acekard-common/IoRpgDefinitions.h"
#include "AkRpgReadSdPatchCode.h"
#include "AkRpgSdReadSectorPatchCode.h"
#include "AkRpgWriteSdPatchCode.h"

/// @brief Implementation of LoaderPlatform for the Acekard 2 flashcard
class AkRpgLoaderPlatform : public IoRpgLoaderPlatform
{
private:
    enum
    {
        IORPG_CMD_SDIO_BYTE = 0xD5
    };

public:
    AkRpgLoaderPlatform() : IoRpgLoaderPlatform(IORPG_CMD_SDIO_BYTE) { }

    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new AkRpgReadSdPatchCode(patchHeap,
                CreateSdHelperPatchCode(patchCodeCollection, patchHeap),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new AkRpgSdReadSectorPatchCode(
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
            return new AkRpgWriteSdPatchCode(patchHeap,
                CreateSdHelperPatchCode(patchCodeCollection, patchHeap)
            );
        });
    }

    void PatchSdscShift(void) const override
    {
        akrpg_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R4, THUMB_R0);
        akrpg_writeSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
    }

    const IoRpgPlatformSpecifics& GetPlatformSpecifics(void) const override
    {
        static const IoRpgPlatformSpecifics data
        {
            .cmd12Command = 0x0C0001D5,
            .cmd17Command = 0x110003D5,
            .cmd18Command = 0x120004D5,
            .cmd24Command = 0x180005D5,
            .sdStateShift = 4
        };
        return data;
    }
};
