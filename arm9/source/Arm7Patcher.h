#pragma once

class LoaderPlatform;

/// @brief Class for patching the arm7 of retail roms.
class Arm7Patcher
{
public:
    /// @brief Applies arm7 patches using the given \p loaderPlatform.
    /// @param loaderPlatform The loader platform to use.
    /// @param cheatsLength The length of the cheats data, or zero when there are no cheats.
    /// @param cheatsPtr Pointer to where the cheats need to be stored, or \c nullptr when there are no cheats.
    /// @param bannerSavePathPtr Pointer to where the banner save path needs to be stored, or \c nullptr when there is no banner save.
    /// @param runInDSiMode \c true if the rom should be loaded in DSi mode, or \c false otherwise.
    /// @return A pointer to the patch space in IWRAM, or \c nullptr if the patches have been placed in main memory.
    void* ApplyPatches(const LoaderPlatform* loaderPlatform, u32 cheatsLength,
        void*& cheatsPtr, char*& bannerSavePathPtr, bool runInDSiMode) const;
};
