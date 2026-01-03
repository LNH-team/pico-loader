#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(r4_writesd);

extern "C" void r4_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class R4WriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit R4WriteSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(r4_writesd), SECTION_SIZE(r4_writesd), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)r4_writeSd);
    }
};
