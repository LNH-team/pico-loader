#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(melonds_writesd);

extern "C" void melonds_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class MelonDSWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit MelonDSWriteSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(melonds_writesd), SECTION_SIZE(melonds_writesd), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)melonds_writeSd);
    }
};
