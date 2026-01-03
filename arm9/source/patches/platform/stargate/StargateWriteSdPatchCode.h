#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_writesd);

extern "C" void stargate_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class StargateWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit StargateWriteSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(stargate_writesd), SECTION_SIZE(stargate_writesd), patchHeap) { }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)stargate_writeSd);
    }
};
