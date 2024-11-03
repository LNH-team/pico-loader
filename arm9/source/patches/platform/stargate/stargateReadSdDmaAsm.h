#pragma once
#include "sections.h"
#include "../SdReadDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(stargate_readsddma);

extern "C" void stargate_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void stargate_finishReadSdDma(void);

extern u32 stargate_readSdDma_miiCardDmaCopy32Ptr;

class StargateReadSdDmaPatchCode : public SdReadDmaPatchCode
{
public:
    StargateReadSdDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : SdReadDmaPatchCode(SECTION_START(stargate_readsddma), SECTION_SIZE(stargate_readsddma), patchHeap)
    {
        stargate_readSdDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const SdReadDmaFunc GetSdReadDmaFunction() const override
    {
        return (const SdReadDmaFunc)GetAddressAtTarget((void*)stargate_readSdDma);
    }

    const SdReadDmaFinishFunc GetSdReadDmaFinishFunction() const override
    {
        return (const SdReadDmaFinishFunc)GetAddressAtTarget((void*)stargate_finishReadSdDma);
    }
};
