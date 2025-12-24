#pragma once
#include "sections.h"
#include "../SdWritePatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_write);

extern u16 DATEL_writeSectorSdhcLabel;

extern u32 DATEL_SDWriteMultipleSector_SpiSendSDIOCommand;

extern u32 DATEL_SDWriteMultipleSector_WaitSpiByteTimeout;
extern u32 DATEL_SDWriteMultipleSector_ReadSpiByte;

extern "C" void DATEL_SDWriteMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class DATELWriteSdPatchCode : public SdWritePatchCode
{
public:
    explicit DATELWriteSdPatchCode(PatchHeap& patchHeap,
        const DATELReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DATELSendSDIOCommandPatchCode* datelSendSDIOCommandPatchCode)
        : SdWritePatchCode(SECTION_START(datel_write), SECTION_SIZE(datel_write), patchHeap)
        {
            DATEL_SDWriteMultipleSector_SpiSendSDIOCommand = (u32)datelSendSDIOCommandPatchCode->GetSpiSendSDIOCommandFunction();

            DATEL_SDWriteMultipleSector_WaitSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetWaitSpiByteTimeoutFunction();
            DATEL_SDWriteMultipleSector_ReadSpiByte = (u32)datelReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)DATEL_SDWriteMultipleSector);
    }
};
