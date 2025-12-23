#pragma once
#include "sections.h"
#include "../SdReadDmaPatchCode.h"

DEFINE_SECTION_SYMBOLS(dspico_readsdsectordma);
DEFINE_SECTION_SYMBOLS(dspico_readsdsectordma_pollSdDataReady);

extern "C" void dspico_readSdSectorDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);
extern "C" void dspico_readSdSectorDma_pollSdDataReady();
extern "C" void dspico_finishReadSdSectorDma(void);

extern u32 dspico_readSdSectorDma_miiCardDmaCopy32Ptr;
extern u32 dspico_readSdSectorDma_pollSdDataReadyPtr;

class DSPicoReadSdSectorDmaPollSdDataReadyPatchCode : public PatchCode
{
public:
    explicit DSPicoReadSdSectorDmaPollSdDataReadyPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dspico_readsdsectordma_pollSdDataReady), SECTION_SIZE(dspico_readsdsectordma_pollSdDataReady), patchHeap) { }

    const void* GetPollSdDataReadyFunction() const
    {
        return GetAddressAtTarget((void*)dspico_readSdSectorDma_pollSdDataReady);
    }

    const SdReadDmaPatchCode::SdReadDmaFinishFunc GetSdReadDmaFinishFunction() const
    {
        return (const SdReadDmaPatchCode::SdReadDmaFinishFunc)GetAddressAtTarget((void*)dspico_finishReadSdSectorDma);
    }
};

class DSPicoReadSdSectorDmaPatchCode : public SdReadDmaPatchCode
{
public:
    DSPicoReadSdSectorDmaPatchCode(PatchHeap& patchHeap,
        const DSPicoReadSdSectorDmaPollSdDataReadyPatchCode* pollSdDataReadyPatchCode, const void* miiCardDmaCopy32Ptr)
        : SdReadDmaPatchCode(SECTION_START(dspico_readsdsectordma), SECTION_SIZE(dspico_readsdsectordma), patchHeap)
        , _sdReadDmaFinishFunc(pollSdDataReadyPatchCode->GetSdReadDmaFinishFunction())
    {
        dspico_readSdSectorDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
        dspico_readSdSectorDma_pollSdDataReadyPtr = (u32)pollSdDataReadyPatchCode->GetPollSdDataReadyFunction();
    }

    const SdReadDmaFunc GetSdReadDmaFunction() const override
    {
        return (const SdReadDmaFunc)GetAddressAtTarget((void*)dspico_readSdSectorDma);
    }

    const SdReadDmaFinishFunc GetSdReadDmaFinishFunction() const override
    {
        return _sdReadDmaFinishFunc;
    }

private:
    const SdReadDmaFinishFunc _sdReadDmaFinishFunc;
};
