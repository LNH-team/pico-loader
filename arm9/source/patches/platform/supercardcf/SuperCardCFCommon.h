#pragma once
#include "sections.h"
#include "../compactflash-common/CompactFlashLockUnlockPatchCode.h"

DEFINE_SECTION_SYMBOLS(sccf_change_mode);

extern "C" void sccf_lockUnlockCard(bool lock);

class SuperCardCFLockUnlockCardPatchCode : public CompactFlashLockUnlockPatchCode
{
public:
    explicit SuperCardCFLockUnlockCardPatchCode(PatchHeap& patchHeap)
        : CompactFlashLockUnlockPatchCode(SECTION_START(sccf_change_mode), SECTION_SIZE(sccf_change_mode), patchHeap) { }

    const void* GetLockUnlockFunction() const override
    {
        return GetAddressAtTarget((void*)sccf_lockUnlockCard);
    }
};
