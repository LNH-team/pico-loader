#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(sdread);

extern "C" void sdread_asm(u32 srcSector, void* dst, u32 sectorCount);

extern u32 sdread_asm_agbRamPtr;

class IsNitroSdReadPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    IsNitroSdReadPatchCode(PatchHeap& patchHeap, u32 agbRamPtr)
        : PatchCode(SECTION_START(sdread), SECTION_SIZE(sdread), patchHeap)
    {
        sdread_asm_agbRamPtr = agbRamPtr;
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)sdread_asm);
    }
};
