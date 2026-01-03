#pragma once
#include "sections.h"
#include "../PatchCode.h"
#include "../ISectorRemapPatchCode.h"
#include "fileInfo.h"

DEFINE_SECTION_SYMBOLS(romoffsettosdsector);

extern "C" u32 rom_offset_to_sd_sector_asm(u32 romOffset);

extern u32 romoffsettosdsector_fatDataPtr;

class RomOffsetToSdSectorPatchCode : public PatchCode, public ISectorRemapPatchCode
{
public:
    RomOffsetToSdSectorPatchCode(PatchHeap& patchHeap, const rom_file_info_t* fatDataPtr)
        : PatchCode(SECTION_START(romoffsettosdsector), SECTION_SIZE(romoffsettosdsector), patchHeap)
    {
        romoffsettosdsector_fatDataPtr = (u32)fatDataPtr;
    }

    const void* GetRemapFunction() const override
    {
        return GetAddressAtTarget((void*)rom_offset_to_sd_sector_asm);
    }
};
