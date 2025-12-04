#pragma once
#include "sections.h"
#include "../SdReadPatchCode.h"
#include "ezpReadSdDataAsm.h"

DEFINE_SECTION_SYMBOLS(ezp_readsectors);

extern "C" void ezp_readSectors(u32 srcSector, void* dst, u32 sectorCount);

extern "C" u32 ezp_readSectors_readSdData_address;

class EZPReadSectorsPatchCode : public SdReadPatchCode
{
public:
    explicit EZPReadSectorsPatchCode(PatchHeap& patchHeap,
        const EZPReadSDDataPatchCode* ezpReadSdDataPatchCode)
        : SdReadPatchCode(SECTION_START(ezp_readsectors), SECTION_SIZE(ezp_readsectors), patchHeap)
    {
        ezp_readSectors_readSdData_address = (u32)ezpReadSdDataPatchCode->GetReadSDDataFunction();
    }

    const SdReadFunc GetSdReadFunction() const override
    {
        return (const SdReadFunc)GetAddressAtTarget((void*)ezp_readSectors);
    }
};
