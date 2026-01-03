#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "../acekard-common/IoRpgSdWaitForStatePatchCode.h"

DEFINE_SECTION_SYMBOLS(ak2_sdreadsector);

extern "C" void ak2_sdReadSector(u32 srcSector, void* dst, u32 sectorCount);

extern u32 ak2_sdReadSector_sdWaitForState_address;

class Ak2SdReadSectorPatchCode : public PatchCode
{
public:
    explicit Ak2SdReadSectorPatchCode(PatchHeap& patchHeap,
        const IoRpgSdWaitForStatePatchCode* iorpgSdWaitForStatePatchCode)
        : PatchCode(SECTION_START(ak2_sdreadsector), SECTION_SIZE(ak2_sdreadsector), patchHeap)
    {
        ak2_sdReadSector_sdWaitForState_address = (u32)iorpgSdWaitForStatePatchCode->GetSDWaitForStateFunction();
    }

    const void* GetSDReadSectorFunction() const
    {
        return GetAddressAtTarget((void*)ak2_sdReadSector);
    }
};
