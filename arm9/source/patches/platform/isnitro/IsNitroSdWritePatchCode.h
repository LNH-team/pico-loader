#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(sdwrite);

extern "C" void sdwrite_asm(u32 dstSector, const void* src, u32 sectorCount);

extern u32 sdwrite_asm_agbRamPtr;

class IsNitroSdWritePatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    IsNitroSdWritePatchCode(PatchHeap& patchHeap, u32 agbRamPtr)
        : PatchCode(SECTION_START(sdwrite), SECTION_SIZE(sdwrite), patchHeap)
    {
        sdwrite_asm_agbRamPtr = agbRamPtr;
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)sdwrite_asm);
    }
};
