#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "EzpReadSdDataPatchCode.h"

DEFINE_SECTION_SYMBOLS(ezp_readsectors);

extern "C" void ezp_readSectors(u32 srcSector, void* dst, u32 sectorCount);

extern "C" u32 ezp_readSectors_readSdData_address;

class EzpReadSectorsPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit EzpReadSectorsPatchCode(PatchHeap& patchHeap,
        const EzpReadSdDataPatchCode* ezpReadSdDataPatchCode)
        : PatchCode(SECTION_START(ezp_readsectors), SECTION_SIZE(ezp_readsectors), patchHeap)
    {
        ezp_readSectors_readSdData_address = (u32)ezpReadSdDataPatchCode->GetReadSDDataFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ezp_readSectors);
    }
};
