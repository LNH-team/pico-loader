#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(melonds_readsddma);

extern "C" void melonds_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void melonds_readSdDmaFinish(void);
extern "C" u32 melonds_readsddma_miiCardDmaCopy32Ptr;

class MelonDSReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    MelonDSReadSdDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(melonds_readsddma), SECTION_SIZE(melonds_readsddma), patchHeap)
    {
        melonds_readsddma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)melonds_readSdDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)melonds_readSdDmaFinish);
    }
};
