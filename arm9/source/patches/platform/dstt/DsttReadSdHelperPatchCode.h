#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd_helper);

extern "C" void dstt_readSd_applySectorCommand(void);
extern "C" void dstt_readSd_waitDataReady(void);
extern "C" void dstt_readSd_transferData();

extern u16 dstt_readSd_sdsc_shift;

class DsttReadSdHelperPatchCode : public PatchCode
{
public:
    explicit DsttReadSdHelperPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_readsd_helper), SECTION_SIZE(dstt_readsd_helper), patchHeap) { }

    const void* GetApplySectorCommandFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_applySectorCommand);
    }

    const void* GetWaitDataReadyFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_waitDataReady);
    }

    const void* GetTransferDataFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_transferData);
    }
};
