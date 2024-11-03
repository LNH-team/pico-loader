#pragma once
#include "sections.h"
#include "../SdWritePatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_writesd);

extern "C" void stargate_writeSd(u32 dstSector, const void* src, u32 sectorCount);

class StargateWriteSdPatchCode : public SdWritePatchCode
{
public:
    explicit StargateWriteSdPatchCode(PatchHeap& patchHeap)
        : SdWritePatchCode(SECTION_START(stargate_writesd), SECTION_SIZE(stargate_writesd), patchHeap) { }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)stargate_writeSd);
    }
};
