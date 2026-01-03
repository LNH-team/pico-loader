#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(melonds_readsd);

extern "C" void melonds_readSd(u32 srcSector, void* dst, u32 sectorCount);

class MelonDSReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit MelonDSReadSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(melonds_readsd), SECTION_SIZE(melonds_readsd), patchHeap) { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)melonds_readSd);
    }
};
