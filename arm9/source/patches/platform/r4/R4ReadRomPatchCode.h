#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(r4_readrom);

extern "C" void r4_readRom(u32 srcSector, void* dst);

class R4ReadRomPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit R4ReadRomPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(r4_readrom), SECTION_SIZE(r4_readrom), patchHeap)
    { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)r4_readRom);
    }
};
