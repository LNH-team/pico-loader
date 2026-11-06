#pragma once
#include "../Patch.h"

/// @brief Arm7 patch that hooks the SDK irq dispatcher to detect the in-game exit
///        combo, publish it for the arm9 and, when the arm9 side asks for it,
///        quiesce the arm7 and hand it over to Pico Loader. This is what makes an
///        in-game exit work on games that have no OS_ResetSystem of their own.
///        The nested irq handler the SDK switches to while the microphone is
///        sampled bypasses the dispatcher, so its dispatch point is hooked too.
///        Optional: silently not applied when the dispatcher is not found.
class ExitComboArm7Patch : public Patch
{
public:
    /// @param exitCombo Keys held to exit, as the active-low KEY_* mask from the
    ///        loader header. 0, or anything the loader would refuse, keeps the
    ///        built-in L + R + Right + Select.
    explicit ExitComboArm7Patch(u16 exitCombo)
        : _exitCombo(exitCombo) { }

    /// @note Only searches once, so it can be asked before the patch is added to
    ///       a collection that searches again.
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

    bool HasTarget() const { return _dispatchTail != nullptr; }
    bool IsInstalled() const { return _installed != 0; }

private:
    u16 _exitCombo;
    // 32-bit fields only: patches live in vram, where byte writes are lost on DS and DS Lite.
    u32* _dispatchTail = nullptr;
    u32* _prioDispatch = nullptr;
    u32 _prioReturnOffset = 0;
    u32 _searched = 0;
    u32 _installed = 0;
};
