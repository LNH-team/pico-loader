#pragma once
#include "sections.h"
#include "../IReadSectorsDmaPatchCode.h"
#include "Ez5hSendCommandPatchCode.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_read_dma);

extern u16 ez5h_sdhc_readDma_label;
extern u32 ez5h_readDma_sendSDIOCommand;
extern u32 ez5h_readDma_miiCardDmaCopy32Ptr;

extern "C" void ez5h_readDma(u32 sector, u32 prevSector, u32 channel, void* dst);
extern "C" void ez5h_endDma();

class Ez5hReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    Ez5hReadSdDmaPatchCode(PatchHeap& patchHeap,
        const Ez5hSendCommandPatchCode* ez5hSendCommandPatchCode, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(ez5h_read_dma), SECTION_SIZE(ez5h_read_dma), patchHeap)
    {
        ez5h_readDma_sendSDIOCommand = (u32)ez5hSendCommandPatchCode->GetSendSDIOCommandFunction();
        ez5h_readDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)ez5h_readDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)ez5h_endDma);
    }
};
