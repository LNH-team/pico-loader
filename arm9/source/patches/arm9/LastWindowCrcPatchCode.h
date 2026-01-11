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

    const void* GetLastWindowCrcFunction() const
    {
        return GetAddressAtTarget((void*)patch_lastwindowcrc_entry);
    }
};
