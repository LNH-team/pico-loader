#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(ezp_readsectorsdma);

extern "C" void ezp_readSectorsDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void ezp_finishReadSectorsDma(void);

extern u32 ezp_readSectorsDma_miiCardDmaCopy32Ptr;

class EzpReadSectorsDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    explicit EzpReadSectorsDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(ezp_readsectorsdma), SECTION_SIZE(ezp_readsectorsdma), patchHeap)
    {
        ezp_readSectorsDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)ezp_readSectorsDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)ezp_finishReadSectorsDma);
    }
};
