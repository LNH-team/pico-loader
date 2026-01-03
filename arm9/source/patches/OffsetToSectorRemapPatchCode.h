#pragma once
#include "sections.h"
#include "PatchCode.h"
#include "ISectorRemapPatchCode.h"

DEFINE_SECTION_SYMBOLS(offsettosectorremap);

extern "C" u32 offset_to_sector_remap(u32 romOffset);

class OffsetToSectorRemapPatchCode : public PatchCode, public ISectorRemapPatchCode
{
public:
    explicit OffsetToSectorRemapPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(offsettosectorremap), SECTION_SIZE(offsettosectorremap), patchHeap) { }

    const void* GetRemapFunction() const override
    {
        return GetAddressAtTarget((void*)offset_to_sector_remap);
    }
};
