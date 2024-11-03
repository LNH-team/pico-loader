#pragma once
#include "sections.h"
#include "../SdReadPatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_readsd);

extern "C" void stargate_readSd(u32 srcSector, void* dst, u32 sectorCount);

class StargateReadSdPatchCode : public SdReadPatchCode
{
public:
    explicit StargateReadSdPatchCode(PatchHeap& patchHeap)
        : SdReadPatchCode(SECTION_START(stargate_readsd), SECTION_SIZE(stargate_readsd), patchHeap) { }

    const SdReadFunc GetSdReadFunction() const override
    {
        return (const SdReadFunc)GetAddressAtTarget((void*)stargate_readSd);
    }
};
