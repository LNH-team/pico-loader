#pragma once
#include "../Patch.h"
#include "LoaderInfo.h"

/// @brief Arm9 patch to make OS_ResetSystem reboot into Pico Loader.
class OSResetSystemPatch : public Patch
{
public:
    OSResetSystemPatch(const loader_info_t* loaderInfo, bool runInDSiMode)
        : _loaderInfo(loaderInfo), _runInDSiMode(runInDSiMode) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

    void** GetCheatsPointerAtTarget() const
    {
        return _cheatsPointer;
    }

private:
    u32* _osResetSystem = nullptr;
    u16 _hybrid = false;
    u16 _runInDSiMode;
    const loader_info_t* _loaderInfo;
    void** _cheatsPointer = nullptr;
};
