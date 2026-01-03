#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(dspico_readsdsectors);
DEFINE_SECTION_SYMBOLS(dspico_readsdsectors_readdirect);

extern "C" void dspico_readSdSectors(u32 srcSector, void* dst, u32 sectorCount);
extern "C" void dspico_readSdSectorsDirect();

extern u32 dspico_readsdsectors_readDirectAddress;

class DSPicoReadSdSectorsDirectPatchCode : public PatchCode
{
public:
    explicit DSPicoReadSdSectorsDirectPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dspico_readsdsectors_readdirect), SECTION_SIZE(dspico_readsdsectors_readdirect), patchHeap) { }

    const void* GetReadSdSectorsDirectFunction() const
    {
        return GetAddressAtTarget((void*)dspico_readSdSectorsDirect);
    }
};

class DSPicoReadSdSectorsPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    DSPicoReadSdSectorsPatchCode(PatchHeap& patchHeap, const DSPicoReadSdSectorsDirectPatchCode* readSdSectorsDirectPatchCode)
        : PatchCode(SECTION_START(dspico_readsdsectors), SECTION_SIZE(dspico_readsdsectors), patchHeap)
    {
        dspico_readsdsectors_readDirectAddress = (u32)readSdSectorsDirectPatchCode->GetReadSdSectorsDirectFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)dspico_readSdSectors);
    }
};
