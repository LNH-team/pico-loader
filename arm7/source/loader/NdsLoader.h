#pragma once
#include "common.h"
#include "ndsHeader.h"
#include "DsiWareSaveArranger.h"
#include "BootMode.h"
#include "ConsoleRegion.h"
#include "UserLanguage.h"

struct dsi_devicelist_entry_t;

/// @brief Class for loading DS(i) roms.
class NdsLoader
{
public:
    /// @brief Sets the \p romPath.
    /// @param romPath The rom path.
    void SetRomPath(const TCHAR* romPath)
    {
        _romPath = romPath;
    }

    /// @brief Sets the \p savePath to use.
    /// @param savePath The save path to use.
    void SetSavePath(const TCHAR* savePath)
    {
        _savePath = savePath;
    }

    /// @brief Sets the argv arguments to pass to the rom.
    /// @param arguments The argv arguments.
    /// @param argumentsLength The length of the argv arguments.
    void SetArguments(const char* arguments, u32 argumentsLength)
    {
        _arguments = arguments;
        _argumentsLength = argumentsLength;
    }

    /// @brief Loads the rom according to the specified \p bootMode.
    /// @param bootMode The boot mode.
    void Load(BootMode bootMode);

private:
    FIL _romFile;
    const TCHAR* _romPath;
    const TCHAR* _savePath;
    u32 _argumentsLength = 0;
    const char* _arguments = nullptr;
    nds_header_twl_t _romHeader;
    DsiWareSaveResult _dsiwareSaveResult;

    bool IsCloneBootRom(u32 romOffset);
    void ApplyArm7Patches();
    void SetupSharedMemory(u32 cardId, u32 agbMem, u32 resetParam, u32 romOffset, u32 bootType);
    void LoadFirmwareUserSettings();
    bool ShouldAttemptDldiPatch();
    void ClearMainMemory();
    void CreateRomClusterTable();
    bool TryLoadRomHeader(u32 romOffset);
    void HandleCardSave();
    void HandleAntiPiracy();
    void RemapWram();
    bool TryLoadArm9();
    bool TryLoadArm9i();
    bool TryDecryptArm9i();
    bool TryDecryptArm7i();
    bool TryLoadArm7();
    bool TryLoadArm7i();
    void HandleDldiPatching();
    void StartRom(BootMode bootMode);
    void SetupTwlConfig();
    void SetDeviceListEntry(dsi_devicelist_entry_t& deviceListEntry,
        char driveLetter, const char* deviceName, const char* path, u8 flags, u8 accessRights);
    void SetupDsiDeviceList();
    void InsertArgv();
    bool TrySetupDsiWareSave();
    bool TryDecryptSecureArea();
    ConsoleRegion GetRomRegion(u32 gameCode);
    UserLanguage GetLanguageByRomRegion(ConsoleRegion romRegion);
    u32 GetSupportedLanguagesByRegion(ConsoleRegion region);
};
