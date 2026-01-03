#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(ace3ds_readsd);

extern "C" void ace3ds_readSd(u32 srcSector, void* dst, u32 sectorCount);

class Ace3DSReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit Ace3DSReadSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ace3ds_readsd), SECTION_SIZE(ace3ds_readsd), patchHeap) { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ace3ds_readSd);
    }
};
