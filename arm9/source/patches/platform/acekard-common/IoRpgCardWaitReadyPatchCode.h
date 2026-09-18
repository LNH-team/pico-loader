#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_cardwaitready);

extern "C" void iorpg_cardWaitReady(void);

class IoRpgCardWaitReadyPatchCode : public PatchCode
{
public:
    IoRpgCardWaitReadyPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(iorpg_cardwaitready), SECTION_SIZE(iorpg_cardwaitready), patchHeap) { }

    const void* GetCardWaitReadyFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_cardWaitReady);
    }
};
