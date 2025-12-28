#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd_waitdataready);

extern "C" void dstt_readSd_waitDataReady();

class DsttReadSdWaitDataReadyPatchCode : public PatchCode
{
public:
    explicit DsttReadSdWaitDataReadyPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_readsd_waitdataready), SECTION_SIZE(dstt_readsd_waitdataready), patchHeap) { }

    const void* GetWaitDataReadyFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_waitDataReady);
    }
};
