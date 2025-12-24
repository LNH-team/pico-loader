#pragma once
#include "sections.h"
#include "../SdReadDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(ards_ntr_command);
DEFINE_SECTION_SYMBOLS(ards_read_spi);
DEFINE_SECTION_SYMBOLS(ards_spi_send);

extern "C" void ARDS_CycleSpi();

extern "C" u8 ARDS_ReadSpiByte();
extern "C" u8 ARDS_ReadWriteSpiByte(u8 value);
extern "C" u8 ARDS_ReadSpiByteTimeout();
extern "C" bool ARDS_WaitSpiByteTimeout();

extern "C" u8 ARDS_SpiSendSDIOCommandR0(u32 arg, u8 cmd);
extern "C" u8 ARDS_SpiSendSDIOCommand(u32 arg, u8 cmd, int extraBytes);

extern u32 ARDS_CycleSpi_ReadSpiByte;

extern u32 ARDS_SpiSendSDIOCommandR0_CycleSpi;

extern u32 ARDS_SpiSendSDIOCommandR0_ReadWriteSpiByte;
extern u32 ARDS_SpiSendSDIOCommandR0_ReadSpiByteTimeout;

class ARDSReadSpiBytePatchCode : public PatchCode
{
public:
    explicit ARDSReadSpiBytePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ards_read_spi), SECTION_SIZE(ards_read_spi), patchHeap) { }

    const void* GetReadSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_ReadSpiByte);
    }

    const void* GetReadWriteSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_ReadWriteSpiByte);
    }

    const void* GetWaitSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_WaitSpiByteTimeout);
    }

    const void* GetReadSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_ReadSpiByteTimeout);
    }
};

class ARDSCycleSpiPatchCode : public PatchCode
{
public:
    explicit ARDSCycleSpiPatchCode(PatchHeap& patchHeap,
        const ARDSReadSpiBytePatchCode* ardsReadSpiBytePatchCode)
        : PatchCode(SECTION_START(ards_ntr_command), SECTION_SIZE(ards_ntr_command), patchHeap)
        {
            ARDS_CycleSpi_ReadSpiByte = (u32)ardsReadSpiBytePatchCode->GetReadSpiByteFunction();
        }

    const void* GetCycleSpiFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_CycleSpi);
    }
};

class ARDSSendSDIOCommandPatchCode : public PatchCode
{
public:
    ARDSSendSDIOCommandPatchCode(PatchHeap& patchHeap,
        const ARDSReadSpiBytePatchCode* ardsReadSpiBytePatchCode,
        const ARDSCycleSpiPatchCode* ardsReadCycleSpiBytePatchCode)
        : PatchCode(SECTION_START(ards_spi_send), SECTION_SIZE(ards_spi_send), patchHeap)
    {
        ARDS_SpiSendSDIOCommandR0_CycleSpi = (u32)ardsReadCycleSpiBytePatchCode->GetCycleSpiFunction();
    
        ARDS_SpiSendSDIOCommandR0_ReadWriteSpiByte = (u32)ardsReadSpiBytePatchCode->GetReadWriteSpiByteFunction();
        ARDS_SpiSendSDIOCommandR0_ReadSpiByteTimeout = (u32)ardsReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
    }

    const void* GetSpiSendSDIOCommandR0Function() const
    {
        return GetAddressAtTarget((void*)ARDS_SpiSendSDIOCommandR0);
    }

    const void* GetSpiSendSDIOCommandFunction() const
    {
        return GetAddressAtTarget((void*)ARDS_SpiSendSDIOCommand);
    }
};
