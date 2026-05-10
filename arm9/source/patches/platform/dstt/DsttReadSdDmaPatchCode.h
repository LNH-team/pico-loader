#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsDmaPatchCode.h"
#include "DsttSdStopTransmissionPatchCode.h"
#include "DsttReadSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsddma);

extern "C" void dstt_readSdDma(u32 srcSector, u32 previousSrcSector, u32 dmaChannel, void* dst);

extern u32 dstt_readSdDma_applySectorCommand_address;
extern u32 dstt_readSdDma_waitDataReady_address;
extern u32 dstt_readSdDma_stopTransmission_address;
extern u32 dstt_readSdDma_miiCardDmaCopy32Ptr;

class DsttReadSdDmaPatchCode : public PatchCode, public IReadSectorsDmaPatchCode
{
public:
    DsttReadSdDmaPatchCode(PatchHeap& patchHeap, const void* miiCardDmaCopy32Ptr,
        const DsttSdStopTransmissionPatchCode* dsttSdStopTransmissionPatchCode,
        const DsttReadSdHelperPatchCode* dsttReadSdHelperPatchCode)
        : PatchCode(SECTION_START(dstt_readsddma), SECTION_SIZE(dstt_readsddma), patchHeap)
    {
        dstt_readSdDma_miiCardDmaCopy32Ptr = (u32)miiCardDmaCopy32Ptr;
        dstt_readSdDma_stopTransmission_address = (u32)dsttSdStopTransmissionPatchCode->GetStopTransmissionFunction();
        dstt_readSdDma_applySectorCommand_address = (u32)dsttReadSdHelperPatchCode->GetApplySectorCommandFunction();
        dstt_readSdDma_waitDataReady_address = (u32)dsttReadSdHelperPatchCode->GetWaitDataReadyFunction();
        _sdReadDmaFinishFunc = (ReadSectorsDmaFinishFunc)dsttSdStopTransmissionPatchCode->GetStopTransmissionFunction();
    }

    const ReadSectorsDmaFunc GetReadSectorsDmaFunction() const override
    {
        return (const ReadSectorsDmaFunc)GetAddressAtTarget((void*)dstt_readSdDma);
    }

    const ReadSectorsDmaFinishFunc GetReadSectorsDmaFinishFunction() const override
    {
        return _sdReadDmaFinishFunc;
    }

private:
    ReadSectorsDmaFinishFunc _sdReadDmaFinishFunc = 0;
};
