#pragma once
#include "../PatchCode.h"
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_retailreturnhotkey);

extern "C" void patch_retailreturnhotkey_entry(void);

extern u32 patch_retailreturnhotkey_original_instruction0;
extern u32 patch_retailreturnhotkey_original_instruction1;
extern u32 patch_retailreturnhotkey_return_address;
extern u32 patch_retailreturnhotkey_reset_address;

class RetailReturnHotkeyPatchCode : public PatchCode
{
public:
    RetailReturnHotkeyPatchCode(PatchHeap& patchHeap, u32 originalInstruction0, u32 originalInstruction1,
        const void* returnAddress, const void* resetAddress)
        : PatchCode(SECTION_START(patch_retailreturnhotkey), SECTION_SIZE(patch_retailreturnhotkey), patchHeap)
    {
        patch_retailreturnhotkey_original_instruction0 = originalInstruction0;
        patch_retailreturnhotkey_original_instruction1 = originalInstruction1;
        patch_retailreturnhotkey_return_address = (u32)returnAddress;
        patch_retailreturnhotkey_reset_address = (u32)resetAddress;
    }

    const void* GetEntryFunction() const
    {
        return GetAddressAtTarget((void*)patch_retailreturnhotkey_entry);
    }
};
