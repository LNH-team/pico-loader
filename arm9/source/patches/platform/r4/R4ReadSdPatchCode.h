#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(r4_readsd);

extern "C" void r4_readSd(u32 srcSector, void* dst, u32 sectorCount);

class R4ReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit R4ReadSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(r4_readsd), SECTION_SIZE(r4_readsd), patchHeap) { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)r4_readSd);
    }
};
