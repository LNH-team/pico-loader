#pragma once
#include "patches/Patch.h"

/// @brief Arm7 patch for redirecting save reads and writes on SDK 2-4.
class CardiTaskThreadPatch : public Patch
{
public:
    enum class PatchVariant : u16
    {
        None,
        ArmA,
        ArmB,
        ArmC,
        ArmD,
        ArmE,
        ArmF,
        ThumbA,
        ThumbB,
        ThumbC,
        ThumbD,
        ThumbE,
        ThumbF
    };

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _cardiTaskThread = nullptr;
    PatchVariant _patchVariant = PatchVariant::None;

    void ApplyArmPatch(void* patch1Address) const;
    void ApplyThumbPatch(void* patch1Address) const;
};