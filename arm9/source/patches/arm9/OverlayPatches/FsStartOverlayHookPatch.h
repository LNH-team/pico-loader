#pragma once
#include "OverlayHookPatch.h"

/// @brief Arm9 patch to apply patches to overlays when they are loaded.
class FsStartOverlayHookPatch : public OverlayHookPatch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _fsStartOverlay = nullptr;
    u16 _thumb = false;
    u16 _hybrid = false;
    const u32* _foundPattern = nullptr;

    void TryPattern(PatchContext& patchContext, const u32* pattern, u32 startOffset);
};
