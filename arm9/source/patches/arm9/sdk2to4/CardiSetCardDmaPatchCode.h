#pragma once
#include "patches/PatchCode.h"
#include "sections.h"
#include "patches/ISectorRemapPatchCode.h"
#include "patches/platform/IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_cardisetcarddma);

extern "C" u32 patch_cardisetcarddma_entry(void);
extern "C" u32 patch_cardionreadcard_entry(void);

extern u32 patch_cardisetcarddma_cardi_common;
extern u32 patch_cardisetcarddma_rom_offset_to_sd_sector_asm_address;
extern u32 patch_cardisetcarddma_sdreaddma_asm_address;
extern u32 patch_cardionreadcard_osdisableirqmask_address;
extern u32 patch_cardionreadcard_sdreaddma_finish_asm_address;

class CardiSetCardDmaPatchCode : public PatchCode
{
public:
    CardiSetCardDmaPatchCode(PatchHeap& patchHeap, const ISectorRemapPatchCode* sectorRemapPatchCode,
        const IReadSectorsDmaPatchCode* readSectorsDmaPatchCode, const void* cardiCommonPointer,
        const void* osDisableIrqMaskPointer)
        : PatchCode(SECTION_START(patch_cardisetcarddma), SECTION_SIZE(patch_cardisetcarddma), patchHeap)
    {
        patch_cardisetcarddma_cardi_common = (u32)cardiCommonPointer;
        patch_cardisetcarddma_rom_offset_to_sd_sector_asm_address = (u32)sectorRemapPatchCode->GetRemapFunction();
        patch_cardisetcarddma_sdreaddma_asm_address = (u32)readSectorsDmaPatchCode->GetReadSectorsDmaFunction();
        patch_cardionreadcard_osdisableirqmask_address = (u32)osDisableIrqMaskPointer;
        patch_cardionreadcard_sdreaddma_finish_asm_address = (u32)readSectorsDmaPatchCode->GetReadSectorsDmaFinishFunction();
    }

    const void* GetCardiSetCardDmaFunction() const
    {
        return GetAddressAtTarget((void*)patch_cardisetcarddma_entry);
    }

    const void* GetCardiOnReadCardPatchFunction() const
    {
        return GetAddressAtTarget((void*)patch_cardionreadcard_entry);
    }
};
