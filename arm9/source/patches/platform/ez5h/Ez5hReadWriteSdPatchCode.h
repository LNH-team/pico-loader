#pragma once
#include "sections.h"
#include "../IReadSectorsPatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "Ez5hReadSdSectorPatchCode.h"
#include "Ez5hWriteSdSectorPatchCode.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_read_write_sd);

extern u32 ez5h_ez5h_doSDOperation_readSector;
extern u32 ez5h_ez5h_doSDOperation_writeSector;

extern "C" void ez5h_writeMultipleSector(u32 sector, u8 * buffer, u32 num_sectors);
extern "C" void ez5h_readMultipleSector(u32 sector, u8 * buffer, u32 num_sectors);

class Ez5hReadWriteSdPatchCode : public PatchCode, public IReadSectorsPatchCode, public IWriteSectorsPatchCode
{
public:
    Ez5hReadWriteSdPatchCode(PatchHeap& patchHeap,
        const Ez5hWriteSdSectorPatchCode* ez5hWriteSdSectorPatchCode,
        const Ez5hReadSdSectorPatchCode* ez5hReadSdSectorPatchCode)
        : PatchCode(SECTION_START(ez5h_read_write_sd), SECTION_SIZE(ez5h_read_write_sd), patchHeap)
	{
		ez5h_ez5h_doSDOperation_writeSector = (u32)ez5hWriteSdSectorPatchCode->GetWriteSectorFunction();
		ez5h_ez5h_doSDOperation_readSector = (u32)ez5hReadSdSectorPatchCode->GetReadSectorFunction();
	}

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ez5h_readMultipleSector);
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)ez5h_writeMultipleSector);
    }
};
