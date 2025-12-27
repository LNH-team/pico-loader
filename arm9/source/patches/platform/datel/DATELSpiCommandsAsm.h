#pragma once
#include "sections.h"
#include "../SdReadDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_cycle_spi);
DEFINE_SECTION_SYMBOLS(datel_read_spi);
DEFINE_SECTION_SYMBOLS(datel_spi_send);

extern "C" void DATEL_CycleSpi();

extern "C" u8 DATEL_ReadSpiByte();
extern "C" u8 DATEL_ReadWriteSpiByte(u8 value);
extern "C" u8 DATEL_ReadSpiByteTimeout();
extern "C" bool DATEL_WaitSpiByteTimeout();

extern "C" u8 DATEL_SpiSendSDIOCommandR0(u32 arg, u8 cmd);
extern "C" u8 DATEL_SpiSendSDIOCommand(u32 arg, u8 cmd, int extraBytes);

extern u32 DATEL_SpiSendSDIOCommandR0_CycleSpi;

extern u32 DATEL_SpiSendSDIOCommandR0_ReadWriteSpiByte;
extern u32 DATEL_SpiSendSDIOCommandR0_ReadSpiByteTimeout;

class DATELReadSpiBytePatchCode : public PatchCode
{
public:
    explicit DATELReadSpiBytePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(datel_read_spi), SECTION_SIZE(datel_read_spi), patchHeap) { }

    const void* GetReadSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_ReadSpiByte);
    }

    const void* GetReadWriteSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_ReadWriteSpiByte);
    }

    const void* GetWaitSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_WaitSpiByteTimeout);
    }

    const void* GetReadSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_ReadSpiByteTimeout);
    }
};

class DATELCycleSpiPatchCode : public PatchCode
{
public:
    explicit DATELCycleSpiPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(datel_cycle_spi), SECTION_SIZE(datel_cycle_spi), patchHeap) { }

    const void* GetCycleSpiFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_CycleSpi);
    }
};

class DATELSendSDIOCommandPatchCode : public PatchCode
{
public:
    DATELSendSDIOCommandPatchCode(PatchHeap& patchHeap,
        const DATELReadSpiBytePatchCode* datelReadSpiBytePatchCode,
        const DATELCycleSpiPatchCode* datelReadCycleSpiBytePatchCode)
        : PatchCode(SECTION_START(datel_spi_send), SECTION_SIZE(datel_spi_send), patchHeap)
    {
        DATEL_SpiSendSDIOCommandR0_CycleSpi = (u32)datelReadCycleSpiBytePatchCode->GetCycleSpiFunction();
    
        DATEL_SpiSendSDIOCommandR0_ReadWriteSpiByte = (u32)datelReadSpiBytePatchCode->GetReadWriteSpiByteFunction();
        DATEL_SpiSendSDIOCommandR0_ReadSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
    }

    const void* GetSpiSendSDIOCommandR0Function() const
    {
        return GetAddressAtTarget((void*)DATEL_SpiSendSDIOCommandR0);
    }

    const void* GetSpiSendSDIOCommandFunction() const
    {
        return GetAddressAtTarget((void*)DATEL_SpiSendSDIOCommand);
    }
};
