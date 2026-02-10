#pragma once
#include "patches/Patch.h"

/// @brief Arm9 patch to redirect DMA card reads, or to disable them entirely, on SDK 2-4.
class CardiTryReadCardDmaPatch : public Patch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _cardiTryReadCardDma = nullptr;
    u32 _thumb = false;
    const u32* _foundPattern = nullptr;

    void TryPattern(PatchContext& patchContext, const u32* pattern);
    void ApplyReturnFalsePatch();
};
