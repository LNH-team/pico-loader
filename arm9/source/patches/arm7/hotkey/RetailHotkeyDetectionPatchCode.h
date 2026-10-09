#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_retailhotkeydetect);

extern "C" void patch_retailhotkeydetect_entry_arm(void);
extern "C" void patch_retailhotkeydetect_entry(void);
extern "C" u32 patch_retailhotkeydetect_loaderExtents[16];
extern "C" u32 patch_retailhotkeydetect_loaderParamsAddress;
extern "C" u32 patch_retailhotkeydetect_dldiAddress;

class RetailHotkeyDetectionPatchCode : public PatchCode
{
public:
    RetailHotkeyDetectionPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(patch_retailhotkeydetect), SECTION_SIZE(patch_retailhotkeydetect), patchHeap)
    {
    }

    const void* GetEntryFunction() const
    {
        return GetAddressAtTarget((void*)patch_retailhotkeydetect_entry);
    }

    const void* GetEntryFunctionArm() const
    {
        return GetAddressAtTarget((void*)patch_retailhotkeydetect_entry_arm);
    }
};
