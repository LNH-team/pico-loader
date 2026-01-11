#pragma once
#include "../PatchCode.h"
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_lastwindowcrc);

extern "C" void patch_lastwindowcrc_entry(void);

class LastWindowCrcPatchCode : public PatchCode
{
public:
    LastWindowCrcPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(patch_lastwindowcrc), SECTION_SIZE(patch_lastwindowcrc), patchHeap)
    { }

    const u32 MakeLastWindowCrcBlx(u32 calleeAddr) const
    {
        u32 addr = (u32)GetAddressAtTarget((void*)patch_lastwindowcrc_entry);
        return 0xFA000000 | (((addr - calleeAddr - 8) >> 2) & 0xFFFFFF);
    }
};
