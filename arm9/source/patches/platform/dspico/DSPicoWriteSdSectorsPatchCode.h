#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(dspico_writesdsectors);

extern "C" void dspico_writeSdSectors(u32 dstSector, const void* src, u32 sectorCount);

class DSPicoWriteSdSectorsPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit DSPicoWriteSdSectorsPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dspico_writesdsectors), SECTION_SIZE(dspico_writesdsectors), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)dspico_writeSdSectors);
    }
};
