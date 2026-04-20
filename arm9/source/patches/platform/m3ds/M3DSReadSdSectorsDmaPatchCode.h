#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(m3ds_readsdsectorsdma);
DEFINE_SECTION_SYMBOLS(m3ds_readsdsectorsdma_helper);

extern "C" void m3ds_readSdSectorsDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void m3ds_finishReadSdSectorsDma(void);
extern "C" void m3ds_readSdSectorsDma_applyCommand(void);
extern "C" void m3ds_sdStopTransmission(void);

extern u32 m3ds_readSdSectorsDma_miiCardDmaCopy32Ptr;
extern u32 m3ds_readSdSectorsDma_applyCommand_address;
extern u32 m3ds_readSdSectorsDma_sdStopTransmission_address;

class M3DSReadSdSectorsDmaHelperPatchCode : public PatchCode
{
public:
    explicit M3DSReadSdSectorsDmaHelperPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(m3ds_readsdsectorsdma_helper),
            SECTION_SIZE(m3ds_readsdsectorsdma_helper), patchHeap) { }

    const void* GetReadSdSectorsDmaApplyCommandFunction() const
    {
        return (const void*)GetAddressAtTarget((void*)m3ds_readSdSectorsDma_applyCommand);
    }

    const void* GetSdStopTransmissionFunction() const
    {
        return (const void*)GetAddressAtTarget((void*)m3ds_sdStopTransmission);
    }
};

class M3DSReadSdSectorsDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    M3DSReadSdSectorsDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr,
        const M3DSReadSdSectorsDmaHelperPatchCode* m3dsReadSdSectorsDmaHelperPatchCode)
        : PatchCode(SECTION_START(m3ds_readsdsectorsdma), SECTION_SIZE(m3ds_readsdsectorsdma), patchHeap)
    {
        m3ds_readSdSectorsDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
        m3ds_readSdSectorsDma_applyCommand_address
            = (u32)m3dsReadSdSectorsDmaHelperPatchCode->GetReadSdSectorsDmaApplyCommandFunction();
        m3ds_readSdSectorsDma_sdStopTransmission_address
            = (u32)m3dsReadSdSectorsDmaHelperPatchCode->GetSdStopTransmissionFunction();
        _sdReadDmaFinishFunc = (ReadSectorsDmaFinishFunc)m3dsReadSdSectorsDmaHelperPatchCode->GetSdStopTransmissionFunction();
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)m3ds_readSdSectorsDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return _sdReadDmaFinishFunc;
    }

private:
    ReadSectorsDmaFinishFunc _sdReadDmaFinishFunc = 0;
};
