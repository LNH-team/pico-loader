#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(ace3ds_writesd);

extern "C" void ace3ds_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class Ace3DSWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit Ace3DSWriteSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ace3ds_writesd), SECTION_SIZE(ace3ds_writesd), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)ace3ds_writeSd);
    }
};
