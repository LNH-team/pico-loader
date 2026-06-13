#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "IoRpgSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_sdreadloop);

extern "C" void iorpg_sdReadLoop(u32 srcSector, void* dst, u32 sectorCount);

extern u32 iorpg_sdReadLoop_sdWaitForState_address;

class IoRpgSdReadLoopPatchCode : public PatchCode
{
public:
    IoRpgSdReadLoopPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode)
        : PatchCode(SECTION_START(iorpg_sdreadloop), SECTION_SIZE(iorpg_sdreadloop), patchHeap)
    {
        iorpg_sdReadLoop_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const void* GetSdReadLoopFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_sdReadLoop);
    }
};
