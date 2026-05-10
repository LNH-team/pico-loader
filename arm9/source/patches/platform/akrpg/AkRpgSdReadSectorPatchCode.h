#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(akrpg_sdreadsector);

extern "C" void akrpg_sdReadSector(u32 srcSector, void* dst, u32 sectorCount);

extern u32 akrpg_sdReadSector_sdWaitForState_address;

class AkRpgSdReadSectorPatchCode : public PatchCode
{
public:
    explicit AkRpgSdReadSectorPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode)
        : PatchCode(SECTION_START(akrpg_sdreadsector), SECTION_SIZE(akrpg_sdreadsector), patchHeap)
    {
        akrpg_sdReadSector_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const void* GetSDReadSectorFunction() const
    {
        return GetAddressAtTarget((void*)akrpg_sdReadSector);
    }
};
