#pragma once
#include "../Patch.h"

/// @brief Arm7 patch to redirecting sound folder to our sub directory.
class RedirectSoundFolderArm7Patch : public Patch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _shared2PathOffset = nullptr;
};
