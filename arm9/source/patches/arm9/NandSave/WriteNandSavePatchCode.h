#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "patches/ISectorRemapPatchCode.h"
#include "patches/platform/IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_writenandsave);

extern "C" bool patch_writeNandSave(const void* src, u32 nandByteOffset, u32 byteLength, u32 dmaChannel);

extern u32 patch_writeNandSave_save_offset_to_sd_sector_asm_address;
extern u32 patch_writeNandSave_sdwrite_asm_address;

class WriteNandSavePatchCode : public PatchCode
{
public:
    WriteNandSavePatchCode(PatchHeap& patchHeap, const ISectorRemapPatchCode* sectorRemapPatchCode,
        const IWriteSectorsPatchCode* writeSectorsPatchCode)
        : PatchCode(SECTION_START(patch_writenandsave), SECTION_SIZE(patch_writenandsave), patchHeap)
    {
        patch_writeNandSave_save_offset_to_sd_sector_asm_address = (u32)sectorRemapPatchCode->GetRemapFunction();
        patch_writeNandSave_sdwrite_asm_address = (u32)writeSectorsPatchCode->GetWriteSectorFunction();
    }

    const void* GetWriteNandSaveFunction() const
    {
        return GetAddressAtTarget((void*)patch_writeNandSave);
    }
};
