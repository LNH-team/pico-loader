#pragma once
#include "sections.h"
#include "../SdWritePatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_write);

extern u16 datel_writeSectorSdhcLabel;

extern u32 datel_SDWriteMultipleSector_SpiSendSDIOCommand;

extern u32 datel_SDWriteMultipleSector_WaitSpiByteTimeout;
extern u32 datel_SDWriteMultipleSector_ReadSpiByte;

extern "C" void datel_SDWriteMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class DatelWriteSdPatchCode : public SdWritePatchCode
{
public:
    DatelWriteSdPatchCode(PatchHeap& patchHeap,
        const DatelReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DatelSendSDIOCommandPatchCode* datelSendSDIOCommandPatchCode)
        : SdWritePatchCode(SECTION_START(datel_write), SECTION_SIZE(datel_write), patchHeap)
        {
            datel_SDWriteMultipleSector_SpiSendSDIOCommand = (u32)datelSendSDIOCommandPatchCode->GetSpiSendSDIOCommandFunction();

            datel_SDWriteMultipleSector_WaitSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetWaitSpiByteTimeoutFunction();
            datel_SDWriteMultipleSector_ReadSpiByte = (u32)datelReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)datel_SDWriteMultipleSector);
    }
};
