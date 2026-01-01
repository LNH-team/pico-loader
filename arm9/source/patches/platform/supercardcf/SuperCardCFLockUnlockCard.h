#pragma once
#include "sections.h"
#include "../compactflash-common/CompactFlashLockUnlockPatchCode.h"

DEFINE_SECTION_SYMBOLS(sccf_lock_unlock);

extern "C" void sccf_lockUnlockCard(bool lock);

class SuperCardCFLockUnlockCardPatchCode : public CompactFlashLockUnlockPatchCode
{
public:
    explicit SuperCardCFLockUnlockCardPatchCode(PatchHeap& patchHeap)
        : CompactFlashLockUnlockPatchCode(SECTION_START(sccf_lock_unlock), SECTION_SIZE(sccf_lock_unlock), patchHeap) { }

    const void* GetLockUnlockFunction() const override
    {
        return GetAddressAtTarget((void*)sccf_lockUnlockCard);
    }
};
