#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_readsd);

extern "C" void stargate_readSd(u32 srcSector, void* dst, u32 sectorCount);

class StargateReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit StargateReadSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(stargate_readsd), SECTION_SIZE(stargate_readsd), patchHeap) { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)stargate_readSd);
    }
};
