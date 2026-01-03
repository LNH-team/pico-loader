#pragma once
#include "sections.h"
#include "PatchCode.h"
#include "ISectorRemapPatchCode.h"
#include "fileInfo.h"

DEFINE_SECTION_SYMBOLS(saveoffsettosdsector);

extern "C" u32 save_offset_to_sd_sector_asm(u32 saveOffset);

extern u32 saveoffsettosdsector_fatDataPtr;

class SaveOffsetToSdSectorPatchCode : public PatchCode, public ISectorRemapPatchCode
{
public:
    SaveOffsetToSdSectorPatchCode(PatchHeap& patchHeap, const save_file_info_t* fatDataPtr)
        : PatchCode(SECTION_START(saveoffsettosdsector), SECTION_SIZE(saveoffsettosdsector), patchHeap)
    {
        saveoffsettosdsector_fatDataPtr = (u32)fatDataPtr;
    }

    const void* GetRemapFunction() const override
    {
        return GetAddressAtTarget((void*)save_offset_to_sd_sector_asm);
    }
};
