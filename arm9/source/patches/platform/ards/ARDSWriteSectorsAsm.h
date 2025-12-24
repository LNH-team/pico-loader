#pragma once
#include "sections.h"
#include "../SdWritePatchCode.h"

DEFINE_SECTION_SYMBOLS(ards_write);

extern u16 ARDS_writeSectorSdhcLabel;

extern u32 ARDS_SDWriteMultipleSector_SpiSendSDIOCommand;

extern u32 ARDS_SDWriteMultipleSector_WaitSpiByteTimeout;
extern u32 ARDS_SDWriteMultipleSector_ReadSpiByte;

extern "C" void ARDS_SDWriteMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class ARDSWriteSdPatchCode : public SdWritePatchCode
{
public:
    explicit ARDSWriteSdPatchCode(PatchHeap& patchHeap,
        const ARDSReadSpiBytePatchCode* ardsReadSpiBytePatchCode,
        const ARDSSendSDIOCommandPatchCode* ardsSendSDIOCommandPatchCode)
        : SdWritePatchCode(SECTION_START(ards_write), SECTION_SIZE(ards_write), patchHeap)
        {
            ARDS_SDWriteMultipleSector_SpiSendSDIOCommand = (u32)ardsSendSDIOCommandPatchCode->GetSpiSendSDIOCommandFunction();

            ARDS_SDWriteMultipleSector_WaitSpiByteTimeout = (u32)ardsReadSpiBytePatchCode->GetWaitSpiByteTimeoutFunction();
            ARDS_SDWriteMultipleSector_ReadSpiByte = (u32)ardsReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)ARDS_SDWriteMultipleSector);
    }
};
