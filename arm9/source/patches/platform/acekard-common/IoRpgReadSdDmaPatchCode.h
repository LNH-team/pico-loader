#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"
#include "IoRpgSdHelperPatchCode.h"
#include "IoRpgDefinitions.h"

DEFINE_SECTION_SYMBOLS(iorpg_readsddma);
DEFINE_SECTION_SYMBOLS(iorpg_dmastarttransfer);

extern "C" void iorpg_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void iorpg_finishReadSdDma(void);
extern "C" void iorpg_dmaStartTransfer(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);

extern u32 iorpg_readSdDma_sendSdioCommand_address;
extern u32 iorpg_readSdDma_cardWaitReady_address;
extern u32 iorpg_readSdDma_sdWaitForState_address;
extern u32 iorpg_readSdDma_cmd12_command;
extern u32 iorpg_readSdDma_cmd18_command;
extern u16 iorpg_readSdDma_sdsc_shift;
extern u32 iorpg_readSdDma_dmaStartTransfer_address;

extern u32 iorpg_dmaStartTransfer_miiCardDmaCopy32Ptr;

class IoRpgDmaStartTransferPatchCode : public PatchCode
{
public:
    IoRpgDmaStartTransferPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr)
        : PatchCode(SECTION_START(iorpg_dmastarttransfer), SECTION_SIZE(iorpg_dmastarttransfer), patchHeap)
    {
        iorpg_dmaStartTransfer_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
    }

    const void* GetDmaStartTransferFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_dmaStartTransfer);
    }
};

class IoRpgReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    IoRpgReadSdDmaPatchCode(PatchHeap& patchHeap,
        const IoRpgCardWaitReadyPatchCode* iorpgCardWaitReadyPatchCode,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgDmaStartTransferPatchCode* IoRpgDmaStartTransferPatchCode,
        const IoRpgPlatformSpecifics& platformSpecifics)
        : PatchCode(SECTION_START(iorpg_readsddma), SECTION_SIZE(iorpg_readsddma), patchHeap)
    {
        iorpg_readSdDma_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        iorpg_readSdDma_cardWaitReady_address = (u32)iorpgCardWaitReadyPatchCode->GetCardWaitReadyFunction();
        iorpg_readSdDma_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
        iorpg_readSdDma_dmaStartTransfer_address = (u32)IoRpgDmaStartTransferPatchCode->GetDmaStartTransferFunction();
        iorpg_readSdDma_cmd12_command = platformSpecifics.cmd12Command;
        iorpg_readSdDma_cmd18_command = platformSpecifics.cmd18Command;
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)iorpg_readSdDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return (const ReadSectorsDmaFinishFunc)GetAddressAtTarget((void*)iorpg_finishReadSdDma);
    }
};
