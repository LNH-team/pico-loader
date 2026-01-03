#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_sdwaitforstate);

extern "C" void iorpg_sdWaitForState(u32 status_shift, u8 state);

class IoRpgSdWaitForStatePatchCode : public PatchCode
{
public:
    explicit IoRpgSdWaitForStatePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(iorpg_sdwaitforstate), SECTION_SIZE(iorpg_sdwaitforstate), patchHeap) { }

    const void* GetSDWaitForStateFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_sdWaitForState);
    }
};
