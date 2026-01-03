#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(g003_writesd);

extern "C" void g003_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class G003WriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit G003WriteSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(g003_writesd), SECTION_SIZE(g003_writesd), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)g003_writeSd);
    }
};
