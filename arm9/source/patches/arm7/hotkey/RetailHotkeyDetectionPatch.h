#pragma once
#include "patches/Patch.h"

/// Optional ARM7 VBlank hook used to prove the retail hotkey can be read
/// without touching the ARM9 IRQ dispatcher. Holding L+R+Down+Select for
/// roughly half a second mutes audio as the detection signal.
class RetailHotkeyDetectionPatch : public Patch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _vblankIrqHandler = nullptr;
    const u32* _foundPattern = nullptr;
    bool _thumb = false;
};
