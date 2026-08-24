#pragma once
#include "patches/Patch.h"

/// @brief Patch that patches sdk5 TWL games so that they keep the SCFG7 register unlocked.
class Scfg7ClearPatch : public Patch
{
public:
    explicit Scfg7ClearPatch() { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _patchLocation = nullptr;
};
