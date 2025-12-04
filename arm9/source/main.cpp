#include "common.h"
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
#include "arm9Clock.h"
#include "errorDisplay/ErrorDisplay.h"
#include "LoaderInfo.h"
#include "jumpToArm9EntryPoint.h"

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
static loader_info_t sLoaderInfo;

u16 gIsDsiMode;

// Dummy symbol to allow linking C++ applications. This is only needed to handle
// dynamic shared objects (.so), but they don't exist on the NDS.
void *__dso_handle;

static u32 receiveFromArm7()
{
    while (ipc_isRecvFifoEmpty());
    return ipc_recvWordDirect();
}

extern "C" void __libc_init_array();

static void clearGraphicsMemory()
{
    // VRAM A used for arm9 code
    mem_setVramBMapping(MEM_VRAM_AB_LCDC);
    // VRAM C and D used for arm7 code
    mem_setVramEMapping(MEM_VRAM_E_LCDC);
    mem_setVramFMapping(MEM_VRAM_FG_LCDC);
    mem_setVramGMapping(MEM_VRAM_FG_LCDC);
    mem_setVramHMapping(MEM_VRAM_H_LCDC);
    mem_setVramIMapping(MEM_VRAM_I_LCDC);
    fastClear((void*)0x06820000, 0x20000); // VRAM B
    fastClear((void*)0x06880000, 0x24000);
    fastClear((void*)GFX_PLTT_BG_MAIN, 512);
    fastClear((void*)GFX_PLTT_BG_SUB, 512);
    fastClear((void*)GFX_PLTT_OBJ_MAIN, 512);
    fastClear((void*)GFX_PLTT_OBJ_SUB, 512);
    fastClear((void*)GFX_OAM_MAIN, 1024);
    fastClear((void*)GFX_OAM_SUB, 1024);

    // clear the vertex and polygon ram of the 3d engine
    gx_init();
    gx_swapBuffers(GX_XLU_SORT_AUTO, GX_DEPTH_MODE_Z);
    gx_swapBuffers(GX_XLU_SORT_AUTO, GX_DEPTH_MODE_Z);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void bootArm9()
{
    mem_setVramAMapping(MEM_VRAM_AB_LCDC);
    fastClear((void*)0x06800000, 0x20000); // VRAM A
    mem_setVramAMapping(MEM_VRAM_AB_NONE);
    // By now it should be safe to unmap the arm7 memory
    mem_setVramCMapping(MEM_VRAM_C_LCDC);
    mem_setVramDMapping(MEM_VRAM_D_LCDC);
    fastClear((void*)0x06840000, 0x40000); // VRAM C and D
    mem_setVramCMapping(MEM_VRAM_C_NONE);
    mem_setVramDMapping(MEM_VRAM_D_NONE);

    if (((REG_SCFG_EXT >> 14) & 3) == 0)
    {
        // When switched to DS mode, disable vram extensions
        REG_SCFG_EXT &= ~(1 << 13);
    }

    while (gfx_getVCount() != 191);
    while (gfx_getVCount() == 191);
    REG_IF = ~0u; // final clear of REG_IF bits
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    jumpToArm9EntryPoint((void*)romHeader->arm9EntryAddress);
}

static void handleInitializeSdCardCommand()
{
    REG_EXMEMCNT &= ~0x0880; // map ds and gba slot to arm9
    bool result = sLoaderPlatform->InitializeSdCard();
    REG_EXMEMCNT |= 0x0880; // map ds and gba slot to arm7
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
    Arm9Patcher().ApplyPatches(
        sLoaderPlatform,
        sApListEntry.GetGameCode() == 0 ? nullptr : &sApListEntry,
        sIsCloneBootRom,
        &sLoaderInfo);
    ipc_sendWordDirect(1);
}

static void handleApplyArm7PatchesCommand()
{
    void* patchSpaceStart = Arm7Patcher().ApplyPatches(sLoaderPlatform);
    ipc_sendWordDirect((u32)patchSpaceStart);
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
    sIsCloneBootRom = receiveFromArm7();
}

static void handleInitializeLoaderInfoCommand()
{
    dc_invalidateRange(TWL_SHARED_MEMORY->ntrSharedMem.cardRomHeader, sizeof(loader_info_t));
    memcpy(&sLoaderInfo, TWL_SHARED_MEMORY->ntrSharedMem.cardRomHeader, sizeof(loader_info_t));
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
        *(vu32*)0x037F8000 = (u32)sdReadPatchCode->GetSdReadFunction();
        *(vu32*)0x037F8004 = (u32)sdWritePatchCode->GetSdWriteFunction();
        patchCodeCollection.CopyAllToTarget();
    }
    dc_flushAll();
    dc_drainWriteBuffer();
    mem_setNtrWramMapping(MEM_NTR_WRAM_ARM7, MEM_NTR_WRAM_ARM7);
    ipc_sendWordDirect(1);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void handleSwitchToDSModeCommand()
{
    Arm9IoRegisterClearer().ClearTwlIoRegisters();
    scfg_setArm9Clock(ScfgArm9Clock::Nitro67MHz);
    REG_SCFG_EXT = 0x83000000u | (1 << 13); // keep vram extensions on until the very end
    ipc_sendWordDirect(1);
}

[[gnu::noinline, gnu::section(".itcm")]]
static void handleBootCommand()
{
    bool isSdkResetSystem = receiveFromArm7() != 0;
    REG_EXMEMCNT &= ~0x0880; // map ds and gba slot to arm9
    sLoaderPlatform->PrepareRomBoot(sRomDirSector, sRomDirSectorOffset);
    Arm9IoRegisterClearer().ClearNtrIoRegisters(isSdkResetSystem);
    auto ntrRomHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    if (ntrRomHeader->IsTwlRom())
    {
        if (gIsDsiMode)
        {
            Arm9IoRegisterClearer().ClearTwlIoRegisters();
            REG_SCFG_EXT = 0x8307F100;
            scfg_setArm9Clock(ScfgArm9Clock::Twl134MHz);
            REG_SCFG_CLK = 0x87;
            REG_SCFG_RST = 1;
        }
        auto twlRomHeader = (const nds_header_twl_t*)TWL_SHARED_MEMORY->twlRomHeader;
        if (twlRomHeader->IsDsiWare())
        {
            REG_EXMEMCNT |= 0x0880; // map ds and gba slot to arm7
        }
    }
    bootArm9();
}

static void handleArm7Command(u32 command)
{
    switch (command)
    {
        case IPC_COMMAND_ARM9_INITIALIZE_SD_CARD:
        {
            handleInitializeSdCardCommand();
            break;
        }
        case IPC_COMMAND_ARM9_WRAM_CONFIG:
        {
            handleWramConfigCommand();
            break;
        }
        case IPC_COMMAND_ARM9_CLEAR_MAIN_MEM:
        {
            handleClearMainMemCommand();
            break;
        }
        case IPC_COMMAND_ARM9_APPLY_PATCHES:
        {
            handleApplyArm9PatchesCommand();
            break;
        }
        case IPC_COMMAND_ARM9_APPLY_ARM7_PATCHES:
        {
            handleApplyArm7PatchesCommand();
            break;
        }
        case IPC_COMMAND_ARM9_SET_AP_INFO:
        {
            handleSetAPInfoCommand();
            break;
        }
        case IPC_COMMAND_ARM9_SET_ROM_FILE_INFO:
        {
            handleSetRomFileInfoCommand();
            break;
        }
        case IPC_COMMAND_ARM9_INITIALIZE_LOADER_INFO:
        {
            handleInitializeLoaderInfoCommand();
            break;
        }
        case IPC_COMMAND_ARM9_GET_SD_FUNCTIONS:
        {
            handleGetSdFunctionsCommand();
            break;
        }
        case IPC_COMMAND_ARM9_DISPLAY_ERROR:
        {
            ErrorDisplay().PrintError((const char*)0x02000000);
            break;
        }
        case IPC_COMMAND_ARM9_SWITCH_TO_DS_MODE:
        {
            handleSwitchToDSModeCommand();
            break;
        }
        case IPC_COMMAND_ARM9_BOOT:
        {
            handleBootCommand();
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

    REG_EXMEMCNT |= 0x8880; // everything to arm7
    // REG_EXMEMCNT &= ~0xFF;
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