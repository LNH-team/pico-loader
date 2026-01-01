#pragma once
#include "sections.h"
#include "../SdReadPatchCode.h"

DEFINE_SECTION_SYMBOLS(m3cf_change_mode);

extern "C" void m3cf_lockUnlockCard(bool lock);

class M3CFChangeModePatchCode : public PatchCode
{
public:
    explicit M3CFChangeModePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(m3cf_change_mode), SECTION_SIZE(m3cf_change_mode), patchHeap) { }

    const void* GetM3LockUnlockCardFunction() const
    {
        return GetAddressAtTarget((void*)m3cf_lockUnlockCard);
    }
};
