#pragma once
#include "sections.h"
#include "../compactflash-common/CompactFlashLockUnlockPatchCode.h"

DEFINE_SECTION_SYMBOLS(m3cf_lock_unlock);

extern "C" void m3cf_lockUnlockCard(bool lock);

class M3CFLockUnlockCardPatchCode : public CompactFlashLockUnlockPatchCode
{
public:
    explicit M3CFLockUnlockCardPatchCode(PatchHeap& patchHeap)
        : CompactFlashLockUnlockPatchCode(SECTION_START(m3cf_lock_unlock), SECTION_SIZE(m3cf_lock_unlock), patchHeap) { }

    const void* GetLockUnlockFunction() const override
    {
        return GetAddressAtTarget((void*)m3cf_lockUnlockCard);
    }
};
