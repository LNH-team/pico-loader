#pragma once
#include "patches/Patch.h"

/// @brief Arm7 patch that applies the game language when the rom's arm7 copies the user settings
///        from the firmware flash to memory (ReadUserInfo in the SDK arm7 components).
class ReadUserInfoLanguagePatch : public Patch
{
public:
    /// @brief Constructs the patch.
    /// @param language The language to write to bits 0-2 of the user settings language field.
    explicit ReadUserInfoLanguagePatch(u32 language)
        : _language(language) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32 _language;
    u32* _copySizeInstruction = nullptr;
    u32 _returnOffset = 0;
};
