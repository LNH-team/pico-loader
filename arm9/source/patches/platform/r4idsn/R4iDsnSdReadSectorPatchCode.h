#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "../acekard-common/IoRpgSdWaitForStatePatchCode.h"

DEFINE_SECTION_SYMBOLS(r4idsn_sdreadsector);

extern "C" void r4idsn_sdReadSector(u32 srcSector, void* dst, u32 sectorCount);

extern u32 r4idsn_sdReadSector_sdWaitForState_address;

class R4iDsnSdReadSectorPatchCode : public PatchCode
{
public:
    explicit R4iDsnSdReadSectorPatchCode(PatchHeap& patchHeap,
        const IoRpgSdWaitForStatePatchCode* iorpgSdWaitForStatePatchCode)
        : PatchCode(SECTION_START(r4idsn_sdreadsector), SECTION_SIZE(r4idsn_sdreadsector), patchHeap)
    {
        r4idsn_sdReadSector_sdWaitForState_address = (u32)iorpgSdWaitForStatePatchCode->GetSDWaitForStateFunction();
    }

    const void* GetSDReadSectorFunction() const
    {
        return GetAddressAtTarget((void*)r4idsn_sdReadSector);
    }
};
