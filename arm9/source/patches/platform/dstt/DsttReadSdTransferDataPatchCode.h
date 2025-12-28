#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd_transferdata);

extern "C" void dstt_readSd_transferData();

class DsttReadSdTransferDataPatchCode : public PatchCode
{
public:
    explicit DsttReadSdTransferDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_readsd_transferdata), SECTION_SIZE(dstt_readsd_transferdata), patchHeap) { }

    const void* GetTransferDataFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_transferData);
    }
};
