#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_readsddma);

extern "C" void stargate_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void stargate_finishReadSdDma(void);

extern u32 stargate_readSdDma_miiCardDmaCopy32Ptr;

class StargateReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    StargateReadSdDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(stargate_readsddma), SECTION_SIZE(stargate_readsddma), patchHeap)
    {
        stargate_readSdDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)stargate_readSdDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)stargate_finishReadSdDma);
    }
};
