#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_read);

extern u16 datel_readSectorSdhcLabel;

extern u32 datel_SDReadMultipleSector_SpiSendSDIOCommandR0;

extern u32 datel_SDReadMultipleSector_ReadSpiByteTimeout;
extern u32 datel_SDReadMultipleSector_ReadSpiShort;

extern "C" void datel_SDReadMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class DatelReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    DatelReadSdPatchCode(PatchHeap& patchHeap,
        const DatelReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DatelSendSdioCommandPatchCode* datelSendSDIOCommandPatchCode)
        : PatchCode(SECTION_START(datel_read), SECTION_SIZE(datel_read), patchHeap)
        {
            datel_SDReadMultipleSector_SpiSendSDIOCommandR0 = (u32)datelSendSDIOCommandPatchCode->GetSpiSendSDIOCommandR0Function();

            datel_SDReadMultipleSector_ReadSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
            datel_SDReadMultipleSector_ReadSpiShort = (u32)datelReadSpiBytePatchCode->GetReadSpiShortFunction();
        }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)datel_SDReadMultipleSector);
    }
};
