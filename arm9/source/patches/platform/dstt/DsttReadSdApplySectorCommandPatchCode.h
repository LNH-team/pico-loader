#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd_applysectorcommand);

extern "C" void dstt_readSd_applySectorCommand();

extern u16 dstt_readSd_sdsc_shift;

class DsttReadSdApplySectorCommandPatchCode : public PatchCode
{
public:
    explicit DsttReadSdApplySectorCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_readsd_applysectorcommand), SECTION_SIZE(dstt_readsd_applysectorcommand), patchHeap) { }

    const void* GetApplySectorCommandFunction() const
    {
        return GetAddressAtTarget((void*)dstt_readSd_applySectorCommand);
    }
};
