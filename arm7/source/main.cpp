#include "common.h"
#include <memory>
#include <string.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/i2c/i2cMcu.h>
#include <libtwl/sio/sioRtc.h>
#include <libtwl/sound/sound.h>
#include <libtwl/sound/soundChannel.h>
#include <libtwl/sound/soundCapture.h>
#include "core/Environment.h"
#include "logger/NitroEmulatorOutputStream.h"
#include "logger/PicoAgbAdapterOutputStream.h"
#include "logger/NocashOutputStream.h"
#include "logger/NullLogger.h"
#include "logger/PlainLogger.h"
#include "fat/dldi.h"
#include "loader/NdsLoader.h"
#include "sharedMemory.h"
#include "ndsHeader.h"
#include "globalHeap.h"
#include "mmc/tmio.h"

#define HANDSHAKE_PART0     0xA
#define HANDSHAKE_PART1     0xB
#define HANDSHAKE_PART2     0xC
#define HANDSHAKE_PART3     0xD

ILogger* gLogger;
FATFS gFatFs;

static NdsLoader sLoader;

static void initLogger()
{
    std::unique_ptr<IOutputStream> outputStream;
    if (Environment::IsIsNitroEmulator() && Environment::SupportsAgbSemihosting())
    {
        outputStream = std::make_unique<NitroEmulatorOutputStream>();
    }
    // else if (Environment::HasPicoAgbAdapter())
    // {
    //     outputStream = std::make_unique<PicoAgbAdapterOutputStream>();
    // }
    else if (Environment::SupportsNocashPrint())
    { 
        outputStream = std::make_unique<NocashOutputStream>();
    }
    else
    {
        gLogger = new NullLogger();
        return;
    }
    gLogger = new PlainLogger(LogLevel::All, std::move(outputStream));
}

static bool mountDldi()
{
    FRESULT res = f_mount(&gFatFs, "fat:", 1);
    if (res != FR_OK)
    {
        LOG_ERROR("dldi mount failed: %d\n", res);
        return false;
    }
    f_chdrive("fat:");
    return true;
}

static bool mountDsiSd()
{
    FRESULT res = f_mount(&gFatFs, "sd:", 1);
    if (res != FR_OK)
    {
        LOG_ERROR("dsi sd mount failed: %d\n", res);
        return false;
    }
    f_chdrive("sd:");
    return true;
}

static bool mountAgbSemihosting()
{
    FRESULT res = f_mount(&gFatFs, "pc2:", 1);
    if (res != FR_OK)
    {
        LOG_ERROR("pc2 sd mount failed: %d\n", res);
        return false;
    }
    f_chdrive("pc2:");
    return true;
}

extern "C" void __libc_init_array();

static void handleSavePath()
{
    if (gLoaderHeader.loadParams.savePath[0] == 0)
    {
        char* savePath = (char*)gLoaderHeader.loadParams.savePath;
        strcpy(savePath, gLoaderHeader.loadParams.romPath);
        char* extension = strrchr(savePath, '.');
        if (!extension)
            extension = &savePath[strlen(savePath)];
        extension[0] = '.';
        extension[1] = 's';
        extension[2] = 'a';
        extension[3] = 'v';
        extension[4] = 0;
    }
    sLoader.SetSavePath(gLoaderHeader.loadParams.savePath);
}

static void clearSoundRegisters()
{
    REG_SOUNDCNT = 0;
    REG_SNDCAP0CNT = 0;
    REG_SNDCAP1CNT = 0;

    for (int i = 0; i < 16; i++)
    {
        REG_SOUNDxCNT(i) = 0;
        REG_SOUNDxSAD(i) = 0;
        REG_SOUNDxTMR(i) = 0;
        REG_SOUNDxPNT(i) = 0;
        REG_SOUNDxLEN(i) = 0;
    }
}

static void initIpc()
{
    ipc_clearSendFifo();
    ipc_ackFifoError();
    ipc_disableRecvFifoNotEmptyIrq();
    ipc_enableFifo();

    while (!ipc_isRecvFifoEmpty())
    {
        ipc_recvWordDirect();
    }

    ipc_setArm7SyncBits(HANDSHAKE_PART0);
    while (ipc_getArm9SyncBits() != HANDSHAKE_PART0);
    ipc_setArm7SyncBits(HANDSHAKE_PART1);
    while (ipc_getArm9SyncBits() != HANDSHAKE_PART1);
    ipc_setArm7SyncBits(HANDSHAKE_PART2);
    while (ipc_getArm9SyncBits() != HANDSHAKE_PART2);
    ipc_setArm7SyncBits(HANDSHAKE_PART3);
    while (ipc_getArm9SyncBits() == HANDSHAKE_PART2);
}

extern "C" void loaderMain()
{
    __libc_init_array();

    clearSoundRegisters();
    rtos_initIrq();
    rtos_startMainThread();
    initIpc();

    bool dsiMode = ipc_getArm9SyncBits() == 1;

    Environment::Initialize(dsiMode);
    heap_init();
    initLogger();

    rtc_init(); // ensure rtc irqs are disabled

    LOG_DEBUG("Pico Loader ARM7 started\n");

    if (Environment::IsDsiMode())
    {
        // Let the mcu handle the power button
        mcu_writeReg(MCU_REG_MODE, 0);
        TMIO_init();
    }

    memset(&gFatFs, 0, sizeof(gFatFs));
    bool multiboot = (gLoaderHeader.bootDrive & PLOAD_BOOT_DRIVE_MULTIBOOT_FLAG) != 0;
    gLoaderHeader.bootDrive &= ~PLOAD_BOOT_DRIVE_MULTIBOOT_FLAG;
    switch (gLoaderHeader.bootDrive)
    {
        case PLOAD_BOOT_DRIVE_DLDI:
        {
            if (dldi_init())
            {
                // The driver cache depends only on whether a driver was handed down,
                // not on where the rom is loaded from, so multiboot needs no special case.
                if (mountDldi())
                {
                    dldi_updateDriverCache();
                }
            }
            break;
        }
        case PLOAD_BOOT_DRIVE_DSI_SD:
        {
            if (Environment::IsDsiMode())
            {
                mountDsiSd();
            }
            break;
        }
        case PLOAD_BOOT_DRIVE_AGB_SEMIHOSTING:
        {
            if (Environment::SupportsAgbSemihosting())
            {
                mountAgbSemihosting();
            }
            break;
        }
    }

    if (gLoaderHeader.v3.cheats != nullptr && gLoaderHeader.v3.cheats->numberOfCheats != 0)
    {
        // Copy cheats to vram
        auto cheats = (pload_cheats_t*)malloc(gLoaderHeader.v3.cheats->length);
        memcpy(cheats, gLoaderHeader.v3.cheats, gLoaderHeader.v3.cheats->length);
        sLoader.SetCheats(cheats);
    }

    if (multiboot)
    {
        LOG_DEBUG("Multiboot\n");
        sLoader.Load(BootMode::Multiboot);
    }
    else if (((nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader)->arm7EntryAddress == (u32)gLoaderHeader.entryPoint)
    {
        LOG_DEBUG("Retail soft reset detected\n");
        u32 originalArm7EntryAddress = ((nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.cardRomHeader)->arm7EntryAddress;
        ((nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader)->arm7EntryAddress = originalArm7EntryAddress;
        sLoader.Load(BootMode::SdkResetSystem);
    }
    else
    {
        sLoader.SetRomPath(gLoaderHeader.loadParams.romPath);
        handleSavePath();
        sLoader.SetArguments(gLoaderHeader.loadParams.arguments, gLoaderHeader.loadParams.argumentsLength);
        sLoader.SetLauncherPath(gLoaderHeader.v2.launcherPath);
        sLoader.Load(BootMode::Normal);
    }

    while (true);
}