#pragma once
#include "sections.h"
#include "../SdWritePatchCode.h"

DEFINE_SECTION_SYMBOLS(ezp_writesectors);

extern "C" void ezp_writeSectors(u32 dstSector, const void* src, u32 sectorCount);

class EZPWriteSectorsPatchCode : public SdWritePatchCode
{
public:
    explicit EZPWriteSectorsPatchCode(PatchHeap& patchHeap)
        : SdWritePatchCode(SECTION_START(ezp_writesectors), SECTION_SIZE(ezp_writesectors), patchHeap) { }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)ezp_writeSectors);
    }
};
