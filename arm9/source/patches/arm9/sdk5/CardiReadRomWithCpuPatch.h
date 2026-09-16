#pragma once
#include "patches/Patch.h"

/// @brief Arm9 patch to redirect card reads on SDK 2-4.
class CardiReadRomWithCpuPatch : public Patch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

    /// @brief Returns the location of CARDi_ReadRomWithCPU. Only valid after FindPatchTarget.
    /// @return The location of CARDi_ReadRomWithCPU, or \c nullptr if it was not found.
    constexpr const u32* GetCardiReadRomWithCpu() const { return _cardiReadRomWithCpu; }

    /// @brief Returns whether CARDi_ReadRomWithCPU is Thumb code. Only valid after FindPatchTarget.
    /// @return True if CARDi_ReadRomWithCPU is Thumb code, false if it is ARM code.
    constexpr bool IsThumb() const { return _thumb; }

private:
    u32* _cardiReadRomWithCpu = nullptr;
    u16 _thumb = false;
    const u32* _foundPattern = nullptr;

    void TryPattern(PatchContext& patchContext, const u32* pattern);
};
