#pragma once
#include "patches/Patch.h"

/// @brief Patch that redirects the banner save file path.
class BannerSavePatch : public Patch
{
public:
    explicit BannerSavePatch(const char* bannerSavePath)
        : _bannerSavePath(bannerSavePath) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _patchLocation = nullptr;
    const u32* _foundSignature = nullptr;
    const char* _bannerSavePath;
};
