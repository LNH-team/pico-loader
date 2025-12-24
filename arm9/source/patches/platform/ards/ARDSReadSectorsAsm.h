#pragma once
#include "sections.h"
#include "../SdReadPatchCode.h"
DEFINE_SECTION_SYMBOLS(ards_read);

extern u16 ARDS_readSectorSdhcLabel;

extern u32 ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0;

extern u32 ARDS_SDReadMultipleSector_ReadSpiByteTimeout;
extern u32 ARDS_SDReadMultipleSector_ReadSpiByte;

extern "C" void ARDS_SDReadMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class ARDSReadSdPatchCode : public SdReadPatchCode
{
public:
    explicit ARDSReadSdPatchCode(PatchHeap& patchHeap,
        const ARDSReadSpiBytePatchCode* ardsReadSpiBytePatchCode,
        const ARDSSendSDIOCommandPatchCode* ardsSendSDIOCommandPatchCode)
        : SdReadPatchCode(SECTION_START(ards_read), SECTION_SIZE(ards_read), patchHeap)
        {
            ARDS_SDReadMultipleSector_SpiSendSDIOCommandR0 = (u32)ardsSendSDIOCommandPatchCode->GetSpiSendSDIOCommandR0Function();

            ARDS_SDReadMultipleSector_ReadSpiByteTimeout = (u32)ardsReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
            ARDS_SDReadMultipleSector_ReadSpiByte = (u32)ardsReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const SdReadFunc GetSdReadFunction() const override
    {
        return (const SdReadFunc)GetAddressAtTarget((void*)ARDS_SDReadMultipleSector);
    }
};
