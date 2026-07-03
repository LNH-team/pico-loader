#pragma once
#include "sections.h"
#include "../IWriteSectorsPatchCode.h"
#include "Ez5hDoSdOperationPatchCode.h"
#include "Ez5hSendCommandPatchCode.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_write_sector);

DEFINE_SECTION_SYMBOLS(ez5h_write_multiple_sector);

DEFINE_SECTION_SYMBOLS(ez5h_crc);

extern u32 ez5h_writeMultipleSector_writeSector_addr;
extern u32 ez5h_writeMultipleSector_doSDOperation;
extern u32 ez5h_writeMultipleSector_sendCommand;
extern u32 ez5h_writeMultipleSector_sdio4BitCrc16;

extern u16 ez5h_sdhc_write_label;

extern "C" bool ez5h_writeSector(u32 sector, void* buffer);

extern "C" void ez5h_sdio4BitCrc16(void*, uint64_t* out);

extern "C" void ez5h_writeMultipleSector(u32 sector, u8 * buffer, u32 num_sectors);

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

class Ez5hWriteMultipleSectorPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    Ez5hWriteMultipleSectorPatchCode(PatchHeap& patchHeap,
		const Ez5hWriteSdSectorPatchCode* ez5hWriteSdSectorPatchCode,
		const Ez5hDoSdOperationPatchCode* ez5hDoSdOperationPatchCode,
		const Ez5hSendCommandPatchCode* ez5hSendCommandPatchCode,
		const Ez5hSdioCrcPatchCode* ez5hSdioCrcPatchCode)
        : PatchCode(SECTION_START(ez5h_write_multiple_sector), SECTION_SIZE(ez5h_write_multiple_sector), patchHeap)
	{
		// we don't want the thumb bit set in the function address, since the address will be fixed accordingly in the code
		ez5h_writeMultipleSector_writeSector_addr = ((u32)ez5hWriteSdSectorPatchCode->GetWriteSectorFunction()) & ((u32)~1);

		ez5h_writeMultipleSector_doSDOperation = (u32)ez5hDoSdOperationPatchCode->GetDoSDOperationFunction();

		ez5h_writeMultipleSector_sendCommand = (u32)ez5hSendCommandPatchCode->GetSendCommandFunction();

		ez5h_writeMultipleSector_sdio4BitCrc16 = (u32)ez5hSdioCrcPatchCode->GetSdioCrcFunction();
	}

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)ez5h_writeMultipleSector);
    }
};
