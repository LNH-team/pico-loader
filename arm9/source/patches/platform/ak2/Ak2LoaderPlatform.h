#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "../acekard-common/IoRpgDefinitions.h"
#include "../acekard-common/IoRpgWriteSdPatchCode.h"
#include "Ak2ReadSdPatchCode.h"
#include "Ak2SdReadSectorPatchCode.h"

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
            return new Ak2ReadSdPatchCode(patchHeap,
                CreateSdHelperPatchCode(patchCodeCollection, patchHeap),
                patchCodeCollection.GetOrAddSharedPatchCode([&]
                {
                    return new Ak2SdReadSectorPatchCode(
                        patchHeap,
                        CreateSdHelperPatchCode(patchCodeCollection, patchHeap)
                    );
                }));
        });
    }

    void PatchSdscShift(void) const override
    {
        ak2_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R4, THUMB_R0);
        iorpg_writeSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
    }

protected:
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
