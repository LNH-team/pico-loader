#pragma once

#define DLDI_MAGIC              0xBF8DA5ED
#define DLDI_DRIVER_MAGIC_NONE  0x49444C44

#define DLDI_FIX_ALL            (1 << 0)
#define DLDI_FIX_GLUE           (1 << 1)
#define DLDI_FIX_GOT            (1 << 2)
#define DLDI_FIX_BSS            (1 << 3)

#define DLDI_FEATURE_CANREAD    (1 << 0)
#define DLDI_FEATURE_CANWRITE   (1 << 1)
#define DLDI_FEATURE_SLOT_GBA   (1 << 4)
#define DLDI_FEATURE_SLOT_NDS   (1 << 5)

typedef bool (*dldi_startup_func_t)(void);
typedef bool (*dldi_inserted_func_t)(void);
typedef bool (*dldi_read_func_t)(u32 sector, u32 count, void* dst);
typedef bool (*dldi_write_func_t)(u32 sector, u32 count, const void* src);
typedef bool (*dldi_clearstatus_func_t)(void);
typedef bool (*dldi_shutdown_func_t)(void);

/// @brief Struct representing a DLDI header.
struct dldi_header_t
{
    u32 dldiMagic;
    u8 dldiString[8];
    u8 dldiVersion;
    u8 driverSize;
    u8 fixFlags;
    u8 stubSize;
    u8 driverName[0x30];
    u32 driverStartAddress;
    u32 driverEndAddress;
    u32 glueStartAddress;
    u32 glueEndAddress;
    u32 gotStartAddress;
    u32 gotEndAddress;
    u32 bssStartAddress;
    u32 bssEndAddress;
    u32 driverMagic;
    u32 featureFlags;
    u32 startupFuncAddress;
    u32 isInsertedFuncAddress;
    u32 readSectorsFuncAddress;
    u32 writeSectorsFuncAddress;
    u32 clearStatusFuncAddress;
    u32 shutdownFuncAddress;
};

static_assert(sizeof(dldi_header_t) == 0x80, "Size of dldi_header_t incorrect");
