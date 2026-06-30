#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_write_sector);

DEFINE_SECTION_SYMBOLS(ez5h_write_data_rom_command);

DEFINE_SECTION_SYMBOLS(ez5h_crc);

extern u16 ez5h_sdhc_write_label;

extern u32 ez5h_writeSector_sendSDIOCommand;
extern u32 ez5h_writeSector_sendCommand;

extern u32 ez5h_writeSector_sendWriteDataRomCommandShort;
extern u32 ez5h_writeSector_sendWriteDataRomCommand;

extern u32 ez5h_writeSector_sdio4BitCrc16;

extern "C" bool ez5h_writeSector(u32 sector, void* buffer);

extern "C" void ez5h_sdio4BitCrc16(void*, uint64_t* out);

extern "C" void ez5h_sendWriteDataRomCommand(const u8* datab);
extern "C" void ez5h_sendWriteDataRomCommandShort(u16 data);

class Ez5hSendWriteDataRomPatchCode : public PatchCode
{
public:
    Ez5hSendWriteDataRomPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ez5h_write_data_rom_command), SECTION_SIZE(ez5h_write_data_rom_command), patchHeap)
        {
        }

    const void* GetSendWriteDataRomFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_sendWriteDataRomCommand);
    }

    const void* GetSendWriteDataRomShortFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_sendWriteDataRomCommandShort);
    }
}

class Ez5hSdioCrcPatchCode : public PatchCode
{
public:
    Ez5hSdioCrcPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ez5h_crc), SECTION_SIZE(ez5h_crc), patchHeap)
        {
        }

    const void* GetSdioCrcFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_sdio4BitCrc16);
    }
}

class Ez5hWriteSdSectorPatchCode : public PatchCode
{
public:
    Ez5hWriteSdPatchCode(PatchHeap& patchHeap,
        const Ez5hSdioCrcPatchCode* ez5hSdioCrcPatchCode,
        const Ez5hSendWriteDataRomPatchCode* ez5hSendWriteDataRomPatchCode,
        const Ez5hSendCommandPatchCode* ez5hSendCommandPatchCode)
        : PatchCode(SECTION_START(ez5h_write_sector), SECTION_SIZE(ez5h_write_sector), patchHeap)
	{
		ez5h_writeSector_sdio4BitCrc16 = (u32)ez5hSdioCrcPatchCode->GetSdioCrcFunction();
		
		ez5h_writeSector_sendCommand = (u32)ez5hSendCommandPatchCode->GetSendCommandFunction();
		ez5h_writeSector_sendSDIOCommand = (u32)ez5hSendCommandPatchCode->GetSendSDIOCommandFunction();
		
		ez5h_writeSector_sendWriteDataRomCommand = (u32)ez5hSendWriteDataRomPatchCode->GetSendWriteDataRomFunction();
		ez5h_writeSector_sendWriteDataRomCommandShort = (u32)ez5hSendWriteDataRomPatchCode->GetSendWriteDataRomShortFunction();
	}

    const void* GetWriteSectorFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_writeSector);
    }
};
