#pragma once
#include <memory>
#include "SdkVersion.h"
#include "AutoloadAdjuster.h"
#include "PatchHeap.h"
#include "PatchCodeCollection.h"

class LoaderPlatform;

/// @brief Class providing a context for performing patches on retail rom arm7 or arm9.
class PatchContext
{
public:
    PatchContext(void* data, u32 dataSize, std::unique_ptr<IAutoloadAdjuster> autoloadAdjuster,
        void* twlData, u32 twlDataSize, std::unique_ptr<IAutoloadAdjuster> twlAutoloadAdjuster,
        SdkVersion sdkVersion, u32 gameCode, u8 gameRevision, const LoaderPlatform* loaderPlatform)
        : _data(data), _dataSize(dataSize), _autoloadAdjuster(std::move(autoloadAdjuster))
        , _twlData(twlData), _twlDataSize(twlDataSize), _twlAutoloadAdjuster(std::move(twlAutoloadAdjuster))
        , _sdkVersion(sdkVersion), _gameCode(gameCode), _gameRevision(gameRevision), _loaderPlatform(loaderPlatform) { }

    /// @brief Tries to find the given \p pattern of the given \p byteLength in the ntr region.
    /// @param pattern The pattern to find.
    /// @param byteLength The length of the pattern.
    /// @return A pointer to the first location where the pattern was found, or \c nullptr if the pattern was not found.
    u32* FindPattern32(const u32* pattern, u32 byteLength) const;

    /// @brief Tries to find the given \p pattern of the given \p byteLength in the twl region.
    /// @param pattern The pattern to find.
    /// @param byteLength The length of the pattern.
    /// @return A pointer to the first location where the pattern was found, or \c nullptr if the pattern was not found.
    u32* FindPattern32Twl(const u32* pattern, u32 byteLength) const;

    /// @brief Returns the ntr autoload adjuster of this context.
    /// @return The ntr autoload adjuster of this context.
    constexpr const IAutoloadAdjuster* GetAutoloadAdjuster() const { return _autoloadAdjuster.get(); }

    /// @brief Returns the twl autoload adjuster of this context.
    /// @return The twl autoload adjuster of this context.
    constexpr const IAutoloadAdjuster* GetAutoloadAdjusterTwl() const { return _twlAutoloadAdjuster.get(); }

    /// @brief Returns the patch heap of this context.
    /// @return The patch heap of this context.
    constexpr PatchHeap& GetPatchHeap() { return _patchHeap; }

    /// @brief Returns the patch code collection of this context.
    /// @return The patch code collection of this context.
    constexpr PatchCodeCollection& GetPatchCodeCollection() { return _patchCodeCollection; }

    /// @brief Returns the SDK version of the rom that is being patched.
    /// @return The SDK version of the rom that is being patched.
    constexpr SdkVersion GetSdkVersion() const { return _sdkVersion; }

    /// @brief Returns the game code of the rom that is being patched.
    /// @return The game code of the rom that is being patched.
    constexpr u32 GetGameCode() const { return _gameCode; }

    /// @brief Returns the game revision of the rom that is being patched.
    /// @return The game revision of the rom that is being patched.
    constexpr u8 GetGameRevision() const { return _gameRevision; }

    /// @brief Returns the loader platform that should be used for the patches.
    /// @return The loader platform that should be used for the patches.
    constexpr const LoaderPlatform* GetLoaderPlatform() { return _loaderPlatform; }

private:
    void* _data;
    u32 _dataSize;
    std::unique_ptr<IAutoloadAdjuster> _autoloadAdjuster;
    void* _twlData;
    u32 _twlDataSize;
    std::unique_ptr<IAutoloadAdjuster> _twlAutoloadAdjuster;
    SdkVersion _sdkVersion;
    u32 _gameCode;
    u8 _gameRevision;

    PatchHeap _patchHeap;
    PatchCodeCollection _patchCodeCollection;
    const LoaderPlatform* _loaderPlatform;
};
