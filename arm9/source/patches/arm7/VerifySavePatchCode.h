#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "patches/ISectorRemapPatchCode.h"
#include "patches/platform/IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(verifysave);

extern "C" void verifysave_asm(u32, u32 saveSrc, void* memoryDst, u32 byteLength);

extern u32 verifysave_tmpBufferPtr;
extern u32 verifysave_save_offset_to_sd_sector_asm_address;
extern u32 verifysave_sdread_asm_address;

class VerifySavePatchCode : public PatchCode
{
public:
    VerifySavePatchCode(PatchHeap& patchHeap, const ISectorRemapPatchCode* sectorRemapPatchCode,
        const IReadSectorsPatchCode* readSectorsPatchCode, void* tmpBuffer)
        : PatchCode(SECTION_START(verifysave), SECTION_SIZE(verifysave), patchHeap)
    {
        verifysave_save_offset_to_sd_sector_asm_address = (u32)sectorRemapPatchCode->GetRemapFunction();
        verifysave_sdread_asm_address = (u32)readSectorsPatchCode->GetReadSectorsFunction();
        verifysave_tmpBufferPtr = (u32)tmpBuffer;
    }

    const void* GetVerifySaveFunction() const
    {
        return GetAddressAtTarget((void*)verifysave_asm);
    }
};
