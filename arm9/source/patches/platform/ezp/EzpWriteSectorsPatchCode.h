#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(ezp_writesectors);

extern "C" void ezp_writeSectors(u32 dstSector, const void* src, u32 sectorCount);

class EzpWriteSectorsPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit EzpWriteSectorsPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ezp_writesectors), SECTION_SIZE(ezp_writesectors), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)ezp_writeSectors);
    }
};
