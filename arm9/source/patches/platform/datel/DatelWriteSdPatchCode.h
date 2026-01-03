#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_write);

extern u16 datel_writeSectorSdhcLabel;

extern u32 datel_SDWriteMultipleSector_SpiSendSDIOCommand;

extern u32 datel_SDWriteMultipleSector_WaitSpiByteTimeout;
extern u32 datel_SDWriteMultipleSector_ReadSpiByte;

extern "C" void datel_SDWriteMultipleSector(u32 srcSector, void* dst, u32 sectorCount);

class DatelWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    DatelWriteSdPatchCode(PatchHeap& patchHeap,
        const DatelReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DatelSendSdioCommandPatchCode* datelSendSDIOCommandPatchCode)
        : PatchCode(SECTION_START(datel_write), SECTION_SIZE(datel_write), patchHeap)
        {
            datel_SDWriteMultipleSector_SpiSendSDIOCommand = (u32)datelSendSDIOCommandPatchCode->GetSpiSendSDIOCommandFunction();

            datel_SDWriteMultipleSector_WaitSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetWaitSpiByteTimeoutFunction();
            datel_SDWriteMultipleSector_ReadSpiByte = (u32)datelReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)datel_SDWriteMultipleSector);
    }
};
