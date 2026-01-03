#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(ace3ds_readsddma);

extern "C" void ace3ds_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void ace3ds_finishReadSdDma(void);

extern u32 ace3ds_readSdDma_miiCardDmaCopy32Ptr;

class Ace3DSReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    Ace3DSReadSdDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(ace3ds_readsddma), SECTION_SIZE(ace3ds_readsddma), patchHeap)
    {
        ace3ds_readSdDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)ace3ds_readSdDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)ace3ds_finishReadSdDma);
    }
};
