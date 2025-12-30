#pragma once
#include "sections.h"
#include "../SdReadDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(datel_read_spi);
DEFINE_SECTION_SYMBOLS(datel_spi_send);

extern "C" u8 datel_readSpiByte();
extern "C" u8 datel_readWriteSpiByte(u8 value);
extern "C" u16 datel_readSpiShort();
extern "C" u8 datel_readSpiByteTimeout();
extern "C" bool datel_waitSpiByteTimeout();

extern "C" u8 datel_spiSendSDIOCommandR0(u32 arg, u8 cmd);
extern "C" u8 datel_spiSendSDIOCommand(u32 arg, u8 cmd, int extraBytes);

extern u32 datel_spiSendSDIOCommandR0_ReadWriteSpiByte;
extern u32 datel_spiSendSDIOCommandR0_ReadSpiByteTimeout;

class DatelReadSpiBytePatchCode : public PatchCode
{
public:
    explicit DatelReadSpiBytePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(datel_read_spi), SECTION_SIZE(datel_read_spi), patchHeap) { }

    const void* GetReadSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)datel_readSpiByte);
    }

    const void* GetReadSpiShortFunction() const
    {
        return GetAddressAtTarget((void*)datel_readSpiShort);
    }

    const void* GetReadWriteSpiByteFunction() const
    {
        return GetAddressAtTarget((void*)datel_readWriteSpiByte);
    }

    const void* GetWaitSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)datel_waitSpiByteTimeout);
    }

    const void* GetReadSpiByteTimeoutFunction() const
    {
        return GetAddressAtTarget((void*)datel_readSpiByteTimeout);
    }
};

class DatelSendSDIOCommandPatchCode : public PatchCode
{
public:
    DatelSendSDIOCommandPatchCode(PatchHeap& patchHeap,
        const DatelReadSpiBytePatchCode* datelReadSpiBytePatchCode)
        : PatchCode(SECTION_START(datel_spi_send), SECTION_SIZE(datel_spi_send), patchHeap)
    {
        datel_spiSendSDIOCommandR0_ReadWriteSpiByte = (u32)datelReadSpiBytePatchCode->GetReadWriteSpiByteFunction();
        datel_spiSendSDIOCommandR0_ReadSpiByteTimeout = (u32)datelReadSpiBytePatchCode->GetReadSpiByteTimeoutFunction();
    }

    const void* GetSpiSendSDIOCommandR0Function() const
    {
        return GetAddressAtTarget((void*)datel_spiSendSDIOCommandR0);
    }

    const void* GetSpiSendSDIOCommandFunction() const
    {
        return GetAddressAtTarget((void*)datel_spiSendSDIOCommand);
    }
};
