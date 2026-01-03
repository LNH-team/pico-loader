#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(g003_readsd);

extern "C" void g003_readSd(u32 srcSector, void* dst, u32 sectorCount);

class G003ReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit G003ReadSdPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(g003_readsd), SECTION_SIZE(g003_readsd), patchHeap) { }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)g003_readSd);
    }
};
