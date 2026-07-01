#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_write_sector);

DEFINE_SECTION_SYMBOLS(ez5h_write_data_rom_command);

DEFINE_SECTION_SYMBOLS(ez5h_crc);

extern u16 ez5h_sdhc_write_label;

extern "C" bool ez5h_writeSector(u32 sector, void* buffer);

extern "C" void ez5h_sdio4BitCrc16(void*, uint64_t* out);

extern "C" void ez5h_sendWriteDataRomCommand(const u8* datab);

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
};

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
};

class Ez5hWriteSdSectorPatchCode : public PatchCode
{
public:
    Ez5hWriteSdSectorPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ez5h_write_sector), SECTION_SIZE(ez5h_write_sector), patchHeap)
	{
	}

    const void* GetWriteSectorFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_writeSector);
    }
};
