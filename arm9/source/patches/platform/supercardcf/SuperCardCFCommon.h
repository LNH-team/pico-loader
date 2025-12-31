#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "../SdReadPatchCode.h"

DEFINE_SECTION_SYMBOLS(sccf_change_mode);

extern "C" void sccf_lockCard();
extern "C" void sccf_unlockCard();

class SuperCardCFChangeModePatchCode : public PatchCode
{
public:
    explicit SuperCardCFChangeModePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sccf_change_mode), SECTION_SIZE(sccf_change_mode), patchHeap) { }

    const void* GetScLockCardFunction() const
    {
        return GetAddressAtTarget((void*)sccf_lockCard);
    }

    const void* GetScUnlockCardFunction() const
    {
        return GetAddressAtTarget((void*)sccf_unlockCard);
    }
};
