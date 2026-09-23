#pragma once

#define IPC_COMMAND_ARM9_WRAM_CONFIG                1
#define IPC_COMMAND_ARM9_CLEAR_MAIN_MEM             2
#define IPC_COMMAND_ARM9_APPLY_PATCHES              3
#define IPC_COMMAND_ARM9_APPLY_ARM7_PATCHES         4
#define IPC_COMMAND_ARM9_SET_AP_INFO                5
#define IPC_COMMAND_ARM9_SET_ROM_FILE_INFO          6
#define IPC_COMMAND_ARM9_INITIALIZE_SD_CARD         8
#define IPC_COMMAND_ARM9_INITIALIZE_LOADER_INFO     9
#define IPC_COMMAND_ARM9_GET_SD_FUNCTIONS           0xA
#define IPC_COMMAND_ARM9_DISPLAY_ERROR              0xB
#define IPC_COMMAND_ARM9_SWITCH_TO_DS_MODE          0xD
#define IPC_COMMAND_ARM9_BOOT                       0xF
#define IPC_COMMAND_ARM9_SETUP_HOMEBREW_BOOTSTUB    0x10

/// @brief Language override value sent with \see IPC_COMMAND_ARM9_APPLY_ARM7_PATCHES when there is no override.
#define IPC_LANGUAGE_OVERRIDE_NONE                  0xFF
