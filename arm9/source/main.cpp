#include "common.h"
extern u32 retailReturnPatchStage;
extern u32 retailReturnFailedAllocation;
#include "ApList.h"
#include <libtwl/gfx/gfx3d.h>
#include <libtwl/gfx/gfx3dCmd.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/mem/memNtrWram.h>
#include <libtwl/mem/memTwlWram.h>
#include <libtwl/mem/memVram.h>
#include "fastClear.h"
#include "cache.h"
#include "sharedMemory.h"
#include "ndsHeader.h"
#include "logger/NocashOutputStream.h"
#include "logger/PicoAgbAdapterOutputStream.h"
#include "logger/PlainLogger.h"
#include "ipcCommands.h"
#include "Arm9IoRegisterClearer.h"
#include "Arm9Patcher.h"
#include "Arm7Patcher.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/platform/LoaderPlatformFactory.h"
#include "patches/arm7/hotkey/RetailHotkeyDetectionPatchCode.h"
#include "arm9Clock.h"
#include "errorDisplay/ErrorDisplay.h"
#include "LoaderInfo.h"
#include "jumpToArm9EntryPoint.h"
#include "patches/homebrew/BootstubPatchCode.h"
#include "HomebrewBootstub.h"

#define HANDSHAKE_PART0     0xA
#define HANDSHAKE_PART1     0xB
#define HANDSHAKE_PART2     0xC
#define HANDSHAKE_PART3     0xD

static NocashOutputStream/*PicoAgbAdapterOutputStream*/ sNocashOutput;
static PlainLogger sPlainLogger { LogLevel::All, &sNocashOutput };
ILogger* gLogger = &sPlainLogger;

static ApListEntry sApListEntry;
static LoaderPlatform* sLoaderPlatform;
static u32 sRomDirSector;
static u32 sRomDirSectorOffset;
static u16 sIsCloneBootRom;
static u16 sRunInDSiMode;
static loader_info_t sLoaderInfo;
extern "C" u32 patch_cardireadcard_loader9Extents[10];
extern "C" u32 patch_cardireadcard_loaderParams[33];
static void** sSoftResetCheatsPointer = nullptr;

u16 gIsDsiMode;
void *__dso_handle;

static u32 receiveFromArm7()
{
    while (ipc_isRecvFifoEmpty());
    return ipc_recvWordDirect();
}

extern "C" void __libc_init_array();

static void clearGraphicsMemory()
{
    mem_setVramBMapping(MEM_VRAM_AB_LCDC);
    mem_setVramEMapping(MEM_VRAM_E_LCDC);
    mem_setVramFMapping(MEM_VRAM_FG_LCDC);
    mem_setVramGMapping(MEM_VRAM_FG_LCDC);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);
    fastClear((void*)0x06820000, 0x20000);
    fastClear((void*)0x06880000, 0x24000);
    fastClear((void*)GFX_PLTT_BG_MAIN, 512);
    fastClear((void*)GFX_PLTT_BG_SUB, 512);
    fastClear((void*)GFX_PLTT_OBJ_MAIN, 512);
    fastClear((void*)GFX_PLTT_OBJ_SUB, 512);
    fastClear((void*)GFX_OAM_MAIN, 1024);
    fastClear((void*)GFX_OAM_SUB, 1024);
    gx_init();
    gx_swapBuffers(GX_XLU_SORT_AUTO, GX_DEPTH_MODE_Z);
    gx_swapBuffers(GX_XLU_SORT_AUTO, GX_DEPTH_MODE_Z);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void bootArm9()
{
    mem_setVramAMapping(MEM_VRAM_AB_LCDC);
    fastClear((void*)0x06800000, 0x20000);
    mem_setVramAMapping(MEM_VRAM_AB_NONE);
    mem_setVramCMapping(MEM_VRAM_C_LCDC);
    mem_setVramDMapping(MEM_VRAM_D_LCDC);
    fastClear((void*)0x06840000, 0x40000);
    mem_setVramCMapping(MEM_VRAM_C_NONE);
    mem_setVramDMapping(MEM_VRAM_D_NONE);
    if (((REG_SCFG_EXT >> 14) & 3) == 0)
        REG_SCFG_EXT &= ~((1 << 13) | (1 << 31));
    while (gfx_getVCount() != 191);
    while (gfx_getVCount() == 191);
    REG_IF = ~0u;
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    jumpToArm9EntryPoint((void*)romHeader->arm9EntryAddress);
}

static void handleInitializeSdCardCommand()
{
    REG_EXMEMCNT &= ~0x0880;
    bool result = sLoaderPlatform->InitializeSdCard();
    REG_EXMEMCNT |= 0x0880;
    ipc_sendWordDirect(result);
}

static void handleWramConfigCommand()
{
    REG_WRAMCNT = receiveFromArm7();
    REG_MBK6 = receiveFromArm7();
    REG_MBK7 = receiveFromArm7();
    REG_MBK8 = receiveFromArm7();
    REG_MBK1 = receiveFromArm7();
    REG_MBK2 = receiveFromArm7();
    REG_MBK3 = receiveFromArm7();
    REG_MBK4 = receiveFromArm7();
    REG_MBK5 = receiveFromArm7();
    ipc_sendWordDirect(1);
}

static void handleClearMainMemCommand()
{
    fastClear((void*)0x02000000, 0x400000);
    dc_flushAll();
    dc_drainWriteBuffer();
    dc_invalidateAll();
    ic_invalidateAll();
    ipc_sendWordDirect(1);
}

static void handleApplyArm9PatchesCommand()
{
    retailReturnPatchStage = 9;
    retailReturnFailedAllocation = 0;
    auto result = Arm9Patcher().ApplyPatches(
        sLoaderPlatform,
        sApListEntry.GetGameCode() == 0 ? nullptr : &sApListEntry,
        sIsCloneBootRom,
        sRunInDSiMode,
        &sLoaderInfo);
    sSoftResetCheatsPointer = result.softResetCheatsPointer;
    ipc_sendWordDirect(1);
}

static void handleApplyArm7PatchesCommand(u32 cheatsLength)
{
    retailReturnPatchStage = 7;
    retailReturnFailedAllocation = 0;
    void* cheats = nullptr;
    char* bannerSavePath = nullptr;
    void* patchSpaceStart = Arm7Patcher().ApplyPatches(sLoaderPlatform, cheatsLength, cheats, bannerSavePath, sRunInDSiMode);
    if (sSoftResetCheatsPointer != nullptr)
        *sSoftResetCheatsPointer = cheats;
    ipc_sendWordDirect((u32)patchSpaceStart);
    ipc_sendWordDirect((u32)cheats);
    ipc_sendWordDirect((u32)bannerSavePath);
    ipc_sendWordDirect(patch_retailhotkeydetect_dldiAddress);
}

static void handleSetAPInfoCommand()
{
    ((u32*)&sApListEntry)[0] = receiveFromArm7();
    ((u32*)&sApListEntry)[1] = receiveFromArm7();
    ((u32*)&sApListEntry)[2] = receiveFromArm7();
    ((u32*)&sApListEntry)[3] = receiveFromArm7();
}

static void handleSetRomFileInfoCommand()
{
    sRomDirSector = receiveFromArm7();
    sRomDirSectorOffset = receiveFromArm7();
    u32 flags = receiveFromArm7();
    sIsCloneBootRom = (flags & 1) != 0;
    sRunInDSiMode = (flags & 2) != 0;
}

static void handleInitializeLoaderInfoCommand()
{
    dc_invalidateRange(TWL_SHARED_MEMORY->ntrSharedMem.cardRomHeader, sizeof(loader_info_t));
    memcpy(&sLoaderInfo, TWL_SHARED_MEMORY->ntrSharedMem.cardRomHeader, sizeof(loader_info_t));

    // Test 107: preserve return target in the relocated ARM9 read patch.
    patch_retailhotkeydetect_loaderParamsAddress = 0;
    patch_cardireadcard_loaderParams[0] = sLoaderInfo.picoLoaderBootDrive & 0x7FFF;
    // Test 116: reclaim 128 game-patch bytes. Never truncate a return path.
    u32 returnPathLength = 0;
    while (returnPathLength < 128 && sLoaderInfo.launcherPath[returnPathLength])
        ++returnPathLength;
    bool returnPathValid = returnPathLength != 0 && returnPathLength < 128;
    memset(&patch_cardireadcard_loaderParams[1], 0, 128);
    if (returnPathValid)
        memcpy(&patch_cardireadcard_loaderParams[1], sLoaderInfo.launcherPath, returnPathLength);
    else
        LOG_DEBUG("Retail hotkey disabled: launcher path exceeds compact capacity or is empty\n");

    // Test 93: bake bounded physical extents for the complete 48480-byte
    // ARM7 loader into the injected patch. No runtime ARM9 mailbox is needed.
    memset(patch_retailhotkeydetect_loaderExtents, 0, 64);
    u32 remainingSectors = 95; // ceil(48480 / 512)
    u32 output = 0;
    if (sLoaderInfo.clusterShift < 32)
    {
        for (u32 i = 1; i + 1 < 16 && remainingSectors != 0; i += 2)
        {
            const u32 count = sLoaderInfo.clusterMap7[i];
            const u32 start = sLoaderInfo.clusterMap7[i + 1];
            if (count == 0 || start < 2)
                break;
            const u64 available = (u64)count << sLoaderInfo.clusterShift;
            const u32 sectors = available < remainingSectors ? (u32)available : remainingSectors;
            const u64 physical = ((u64)(start - 2) << sLoaderInfo.clusterShift) + sLoaderInfo.database;
            if (physical == 0 || physical + sectors - 1 > 0xFFFFFFFFull)
                break;
            patch_retailhotkeydetect_loaderExtents[output++] = sectors;
            patch_retailhotkeydetect_loaderExtents[output++] = (u32)physical;
            remainingSectors -= sectors;
        }
    }
    if (remainingSectors != 0)
        memset(patch_retailhotkeydetect_loaderExtents, 0, 64);

    // Test 106: retain all allocated ARM9-loader sectors, bounded to VRAM A.
    // Read whole clusters so the evolving ARM9 binary needs no fixed size.
    memset(patch_cardireadcard_loader9Extents, 0, 40);
    u32 loader9Sectors = 0;
    u32 loader9Output = 0;
    bool loader9MapValid = sLoaderInfo.clusterShift < 32;
    if (loader9MapValid)
    {
        for (u32 i = 1; i + 1 < 10; i += 2)
        {
            const u32 count = sLoaderInfo.clusterMap9[i];
            if (count == 0)
                break;
            const u32 start = sLoaderInfo.clusterMap9[i + 1];
            const u64 sectors = (u64)count << sLoaderInfo.clusterShift;
            const u64 physical = start >= 2
                ? ((u64)(start - 2) << sLoaderInfo.clusterShift) + sLoaderInfo.database : 0;
            if (sectors == 0 || sectors > 256 - loader9Sectors ||
                physical == 0 || physical + sectors - 1 > 0xFFFFFFFFull)
            {
                loader9MapValid = false;
                break;
            }
            patch_cardireadcard_loader9Extents[loader9Output++] = (u32)sectors;
            patch_cardireadcard_loader9Extents[loader9Output++] = (u32)physical;
            loader9Sectors += (u32)sectors;
        }
        if (sLoaderInfo.clusterMap9[9] != 0)
            loader9MapValid = false;
    }
    if (!loader9MapValid || loader9Sectors == 0)
        memset(patch_cardireadcard_loader9Extents, 0, 40);

    // Disable the ARM7 request before takeover when compact parameters cannot fit.
    if (!returnPathValid)
        memset(patch_retailhotkeydetect_loaderExtents, 0, 64);

    dc_flushAll();
    dc_drainWriteBuffer();
    ipc_sendWordDirect(1);
}

static void handleGetSdFunctionsCommand()
{
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM9, MEM_NTR_WRAM_ARM9);
    PatchHeap patchHeap;
    PatchCodeCollection patchCodeCollection;
    patchHeap.AddFreeSpace((void*)0x037F8020, 16 * 1024);
    {
        auto sdReadPatchCode = sLoaderPlatform->CreateSdReadPatchCode(patchCodeCollection, patchHeap);
        auto sdWritePatchCode = sLoaderPlatform->CreateSdWritePatchCode(patchCodeCollection, patchHeap);
        *(vu32*)0x037F8000 = (u32)sdReadPatchCode->GetReadSectorsFunction();
        *(vu32*)0x037F8004 = (u32)sdWritePatchCode->GetWriteSectorFunction();
        patchCodeCollection.CopyAllToTarget();
    }
    dc_flushAll();
    dc_drainWriteBuffer();
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM7, MEM_NTR_WRAM_ARM7);
    ipc_sendWordDirect(1);
}

static void handleDisplayErrorCommand()
{
    bool isWarning = receiveFromArm7();
    if (isWarning)
        ErrorDisplay().PrintWarning((const char*)0x02000000);
    else
        ErrorDisplay().PrintError((const char*)0x02000000);
    ipc_sendWordDirect(1);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void handleSwitchToDSModeCommand()
{
    Arm9IoRegisterClearer().ClearTwlIoRegisters();
    scfg_setArm9Clock(ScfgArm9Clock::Nitro67MHz);
    REG_SCFG_EXT = 0x83000000u | (1 << 13);
    ipc_sendWordDirect(1);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void handleBootCommand()
{
    bool isSdkResetSystem = receiveFromArm7() != 0;
    REG_EXMEMCNT &= ~0x0880;
    sLoaderPlatform->PrepareRomBoot(sRomDirSector, sRomDirSectorOffset);
    Arm9IoRegisterClearer().ClearNtrIoRegisters(isSdkResetSystem);
    REG_EXMEMCNT |= 0x0880;
    if (sRunInDSiMode)
    {
        Arm9IoRegisterClearer().ClearTwlIoRegisters();
        REG_SCFG_EXT = 0x8307F100;
        scfg_setArm9Clock(ScfgArm9Clock::Twl134MHz);
        REG_SCFG_CLK = 0x87;
        REG_SCFG_RST = 1;
    }
    bootArm9();
}

static void handleSetupHomebrewBootstub(u32 dldiRequiredSpace)
{
    PatchHeap patchHeap;
    PatchCodeCollection patchCodeCollection;
    void* patchSpace = (void*)&HOMEBREW_BOOTSTUB[1];
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    if (!romHeader->SupportsDsiMode())
        patchSpace = (u8*)patchSpace - 0x2F00000 + 0x2300000;
    patchHeap.AddFreeSpace(patchSpace, 32 * 1024 - sizeof(homebrew_bootstub_t));

    void* dldi = patchHeap.Alloc(dldiRequiredSpace);
    char* launcherPath = (char*)patchHeap.Alloc(256);
    auto bootstubPatchCode = patchCodeCollection.AddUniquePatchCode<BootstubPatchCode>(
        patchHeap, dldi, launcherPath, &sLoaderInfo,
        sLoaderPlatform->CreateSdReadPatchCode(patchCodeCollection, patchHeap));

    HOMEBREW_BOOTSTUB->bootSig = HOMEBREW_BOOTSTUB_BOOTSIG;
    HOMEBREW_BOOTSTUB->arm9Reboot = (void*)bootstubPatchCode->GetArm9RebootFunction();
    HOMEBREW_BOOTSTUB->arm7Reboot = (void*)bootstubPatchCode->GetArm7RebootFunction();
    HOMEBREW_BOOTSTUB->bootSize = 32 * 1024 - sizeof(homebrew_bootstub_t);
    patchCodeCollection.CopyAllToTarget();
    ipc_sendWordDirect((u32)dldi);
    ipc_sendWordDirect((u32)launcherPath);
}

static void handleArm7Command(u32 command)
{
    switch (command)
    {
        case IPC_COMMAND_ARM9_INITIALIZE_SD_CARD: handleInitializeSdCardCommand(); break;
        case IPC_COMMAND_ARM9_WRAM_CONFIG: handleWramConfigCommand(); break;
        case IPC_COMMAND_ARM9_CLEAR_MAIN_MEM: handleClearMainMemCommand(); break;
        case IPC_COMMAND_ARM9_APPLY_PATCHES: handleApplyArm9PatchesCommand(); break;
        case IPC_COMMAND_ARM9_APPLY_ARM7_PATCHES:
        {
            u32 cheatsLength = receiveFromArm7();
            handleApplyArm7PatchesCommand(cheatsLength);
            break;
        }
        case IPC_COMMAND_ARM9_SET_AP_INFO: handleSetAPInfoCommand(); break;
        case IPC_COMMAND_ARM9_SET_ROM_FILE_INFO: handleSetRomFileInfoCommand(); break;
        case IPC_COMMAND_ARM9_INITIALIZE_LOADER_INFO: handleInitializeLoaderInfoCommand(); break;
        case IPC_COMMAND_ARM9_GET_SD_FUNCTIONS: handleGetSdFunctionsCommand(); break;
        case IPC_COMMAND_ARM9_DISPLAY_ERROR: handleDisplayErrorCommand(); break;
        case IPC_COMMAND_ARM9_SWITCH_TO_DS_MODE: handleSwitchToDSModeCommand(); break;
        case IPC_COMMAND_ARM9_BOOT: handleBootCommand(); break;
        case IPC_COMMAND_ARM9_SETUP_HOMEBREW_BOOTSTUB:
        {
            u32 dldiRequiredSpace = receiveFromArm7();
            handleSetupHomebrewBootstub(dldiRequiredSpace);
            break;
        }
    }
}

extern "C" void loaderMain()
{
    __libc_init_array();
    clearGraphicsMemory();
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART0);
    ipc_setArm9SyncBits(HANDSHAKE_PART0);
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART1);
    ipc_setArm9SyncBits(HANDSHAKE_PART1);
    REG_EXMEMCNT |= 0x8880;
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM7, MEM_NTR_WRAM_ARM7);
    ipc_clearSendFifo();
    ipc_ackFifoError();
    ipc_disableRecvFifoNotEmptyIrq();
    ipc_enableFifo();
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART2);
    ipc_setArm9SyncBits(HANDSHAKE_PART2);
    while (ipc_getArm7SyncBits() != HANDSHAKE_PART3);
    ipc_setArm9SyncBits((*(vu32*)0x04004000) & 3);
    gIsDsiMode = ((*(vu32*)0x04004000) & 3) == 1;
    heap_init();
    sLoaderPlatform = LoaderPlatformFactory().CreateLoaderPlatform();
    sRomDirSector = 0;
    sRomDirSectorOffset = 0;
    sIsCloneBootRom = false;
    LOG_DEBUG("Pico Loader ARM9 started\n");
    while (true)
    {
        if (ipc_isRecvFifoEmpty())
            continue;
        u32 word = ipc_recvWordDirect();
        handleArm7Command(word);
    }
}
