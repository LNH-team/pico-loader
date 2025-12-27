#pragma once
#include "sections.h"
#include "../SdReadPatchCode.h"
DEFINE_SECTION_SYMBOLS(datel_read);

extern u16 DATEL_readSectorSdhcLabel;

extern u32 DATEL_SDReadMultipleSector_SpiSendSDIOCommandR0;

extern u32 DATEL_SDReadMultipleSector_ReadSpiByteTimeout;
extern u32 DATEL_SDReadMultipleSector_ReadSpiByte;

extern "C" void DATEL_SDReadMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class DATELReadSdPatchCode : public SdReadPatchCode
{
public:
    DATELReadSdPatchCode(PatchHeap& patchHeap,
        const DATELReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DATELSendSDIOCommandPatchCode* datelSendSDIOCommandPatchCode)
        : SdReadPatchCode(SECTION_START(datel_read), SECTION_SIZE(datel_read), patchHeap)
        {
            DATEL_SDReadMultipleSector_SpiSendSDIOCommandR0 = (u32)datelSendSDIOCommandPatchCode->GetSpiSendSDIOCommandR0Function();

            DATEL_SDReadMultipleSector_ReadSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
            DATEL_SDReadMultipleSector_ReadSpiByte = (u32)datelReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const SdReadFunc GetSdReadFunction() const override
    {
        return (const SdReadFunc)GetAddressAtTarget((void*)DATEL_SDReadMultipleSector);
    }
};
