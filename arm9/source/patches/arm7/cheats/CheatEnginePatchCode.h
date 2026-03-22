#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_cheatengine);

extern "C" void cheatengine_entry_arm(void);
extern "C" void cheatengine_entry(void);

extern const void* cheatengine_cheatsPtr;

class CheatEnginePatchCode : public PatchCode
{
public:
    CheatEnginePatchCode(PatchHeap& patchHeap, const void* cheatsAddress)
        : PatchCode(SECTION_START(patch_cheatengine), SECTION_SIZE(patch_cheatengine), patchHeap)
    {
        cheatengine_cheatsPtr = cheatsAddress;
    }

    const void* GetCheatEngineFunction() const
    {
        return GetAddressAtTarget((void*)cheatengine_entry);
    }

    const void* GetCheatEngineFunctionArm() const
    {
        return GetAddressAtTarget((void*)cheatengine_entry_arm);
    }
};
