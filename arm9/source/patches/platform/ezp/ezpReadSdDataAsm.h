#pragma once
#include "sections.h"
#include "thumbInstructions.h"

DEFINE_SECTION_SYMBOLS(ezp_readsddata);

extern "C" void ezp_readSdData(u32 srcSector, void* dst, u32 sectorCount, u32 sectorCountThisRead);

class EZPReadSDDataPatchCode : public PatchCode
{
public:
    explicit EZPReadSDDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ezp_readsddata), SECTION_SIZE(ezp_readsddata), patchHeap) { }

    const void* GetReadSDDataFunction() const
    {
        return GetAddressAtTarget((void*)ezp_readSdData);
    }
};
