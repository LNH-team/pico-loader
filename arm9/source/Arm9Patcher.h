#pragma once
#include "LoaderInfo.h"
#include "SdkVersion.h"

class ApListEntry;
class LoaderPlatform;
class PatchContext;
class PatchCollection;
class OverlayHookPatch;

/// @brief Class for patching the arm9 of retail roms.
class Arm9Patcher
{
public:
    struct PatchResult
    {
        /// @brief Pointer to the cheats pointer used for soft reset.
        ///        This pointer must be set to keep cheats over soft resets.
        void** softResetCheatsPointer;
    };

    /// @brief Applies arm9 patches using the given \p loaderPlatform.
    /// @param loaderPlatform The loader platform to use.
    /// @param apListEntry The AP list entry for the rom being loaded, or \c nullptr if there is none.
    /// @param isCloneBootRom \c true if the rom being loaded is a clone boot rom, or \c false otherwise.
    /// @param runInDSiMode \c true if the rom should be loaded in DSi mode, or \c false otherwise.
    /// @param loaderInfo The loader info to use.
    /// @return Some information resulting from the patching.
    PatchResult ApplyPatches(const LoaderPlatform* loaderPlatform, const ApListEntry* apListEntry,
        bool isCloneBootRom, bool runInDSiMode, const loader_info_t* loaderInfo) const;

private:
    const u32* FindMIiUncompressBackward(u32 arm9LoadAddress, SdkVersion sdkVersion) const;
    void AddGamePatches(PatchCollection& patchCollection, u32 gameCode, const ApListEntry* apListEntry) const;
    void AddDSProtectPatches(PatchCollection& patchCollection,
        OverlayHookPatch* overlayHookPatch, const ApListEntry* apListEntry) const;
    void AddGameSpecificPatches(PatchCollection& patchCollection,
        OverlayHookPatch* overlayHookPatch, u32 gameCode) const;
    void AddRestoreCompressedEndPatch(PatchContext& patchContext,
        u32 arm9AutoLoadDoneHookAddress, u32* moduleParamsCompressedEnd, u32 originalCompressedEndValue) const;
    u32 GetAvailableParentSectionSpace() const;
};
