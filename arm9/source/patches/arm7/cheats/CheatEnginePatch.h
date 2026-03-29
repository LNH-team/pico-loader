#pragma once
#include "patches/Patch.h"

/// @brief Arm7 patch for injecting the cheat engine in the vblank interrupt handler.
class CheatEnginePatch : public Patch
{
public:
    explicit CheatEnginePatch(const void* cheats)
        : _cheats(cheats) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    const void* _cheats;
    u32* _vblankIrqHandler = nullptr;
    const u32* _foundPattern = nullptr;
    u16 _thumb = false;
};
