#include "common.h"
#include <libtwl/mem/memNtrWram.h>
#include "ModuleParamsLocator.h"
#include "SdkVersion.h"
#include "sharedMemory.h"
#include "gameCode.h"
#include "cache.h"
#include "errorDisplay/ErrorDisplay.h"
#include "patches/PatchCollection.h"
#include "patches/PatchContext.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm7/sdk2to4/CardiTaskThreadPatch.h"
#include "patches/arm7/sdk5/CardiDoTaskFromArm9Patch.h"
#include "patches/arm7/OsGetInitArenaLoPatch.h"
#include "patches/arm7/DisableArm7WramClearPatch.h"
#include "patches/arm7/sdk5/Sdk5DsiSdCardRedirectPatch.h"
#include "patches/arm7/PokemonDownloaderArm7Patch.h"
#include "Arm7Patcher.h"

void* Arm7Patcher::ApplyPatches(const LoaderPlatform* loaderPlatform) const
{
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    auto twlRomHeader = (const nds_header_twl_t*)TWL_SHARED_MEMORY->twlRomHeader;
    ModuleParamsLocator moduleParamsLocator;
    auto moduleParams = moduleParamsLocator.FindModuleParams(romHeader);
    SdkVersion sdkVersion = moduleParams ? moduleParams->sdkVersion : 0u;
    if (!moduleParams && romHeader->gameCode == GAMECODE("AS2E"))
    {
        // Spider-Man 2 (USA) is probably the only game without module params
        sdkVersion = 0x02004F50;
    }
    std::unique_ptr<IAutoloadAdjuster> arm7Autoload; // TODO unused
    std::unique_ptr<IAutoloadAdjuster> arm7iAutoload;
    if (gIsDsiMode && romHeader->SupportsDsiMode())
    {
        auto arm7iModuleParams = (const module_params_twl_t*)(twlRomHeader->arm7LoadAddress + twlRomHeader->arm7iModuleParamsAddress);
        arm7iAutoload = std::make_unique<AutoloadAdjuster<autoload_list_entry_sdk5_t>>(
            (autoload_list_entry_sdk5_t*)arm7iModuleParams->autoloadListStart,
            (autoload_list_entry_sdk5_t*)arm7iModuleParams->autoloadListEnd,
            arm7iModuleParams->autoloadStart);
    }
    PatchCollection patchCollection;
    LOG_DEBUG("Arm7 region: 0x%x - 0x%x\n", romHeader->arm7LoadAddress, romHeader->arm7LoadAddress + romHeader->arm7Size);
    PatchContext patchContext
    {
        (void*)romHeader->arm7LoadAddress,
        romHeader->arm7Size,
        std::move(arm7Autoload),
        (romHeader->SupportsDsiMode()) ? (void*)twlRomHeader->arm7iLoadAddress : nullptr,
        (romHeader->SupportsDsiMode()) ? twlRomHeader->arm7iSize : 0,
        std::move(arm7iAutoload),
        sdkVersion,
        romHeader->gameCode,
        romHeader->softwareVersion,
        loaderPlatform
    };
    void* patchSpaceStart = nullptr;
    if (sdkVersion != 0)
    {
        mem_setNtrWramMapping(MEM_NTR_WRAM_ARM9, MEM_NTR_WRAM_ARM9);
        auto arm7ArenaPatch = new OsGetInitArenaLoPatch();
        patchCollection.AddPatch(arm7ArenaPatch);

        if (romHeader->unitCode == 0) // seems only present on NITRO, not on HYBRID or LIMITED
        {
            patchCollection.AddPatch(new DisableArm7WramClearPatch());
        }

        if (sdkVersion.IsTwlSdk())
        {
            if (gIsDsiMode && (twlRomHeader->HasNandAccess() || twlRomHeader->HasSdAccess()))
            {
                patchCollection.AddPatch(new Sdk5DsiSdCardRedirectPatch());
            }

            if (!twlRomHeader->IsDsiWare())
            {
                patchCollection.AddPatch(new CardiDoTaskFromArm9Patch());
            }
        }
        else
        {
            patchCollection.AddPatch(new CardiTaskThreadPatch());
        }

        if (*(vu32*)0x02FFF00C == GAMECODE("ADAJ") &&
            romHeader->arm9LoadAddress == 0x02004000 &&
            romHeader->arm9EntryAddress == 0x02004800)
        {
            // pokemon downloader
            patchCollection.AddPatch(new PokemonDownloaderArm7Patch());
        }

        if (arm7ArenaPatch->FindPatchTarget(patchContext))
        {
            const u32 arm7PatchSpaceSize = 0x800;
            void* privateWramHeapStart = arm7ArenaPatch->GetArm7PrivateWramArenaLo();
            if (0x0380F780 - (u32)privateWramHeapStart - 0x2100 >= arm7PatchSpaceSize)
            {
                patchSpaceStart = privateWramHeapStart;
                arm7ArenaPatch->SetArm7PrivateWramArenaLo((u8*)patchSpaceStart + arm7PatchSpaceSize);
            }
            else
            {
                LOG_DEBUG("Arm 7 patches placed in main memory\n");
                u32 mainMemoryArenaLo = (u32)arm7ArenaPatch->GetMainMemoryArenaLo();
                if (gIsDsiMode && romHeader->unitCode == 0)
                {
                    patchSpaceStart = (void*)(mainMemoryArenaLo & ~0xC00000); // 0x023...
                }
                else
                {
                    patchSpaceStart = (void*)(mainMemoryArenaLo | 0x800000); // make sure it ends up in the right place while in 16MB mode
                }
                arm7ArenaPatch->SetMainMemoryArenaLo((u8*)mainMemoryArenaLo + arm7PatchSpaceSize);
            }
            patchContext.GetPatchHeap().AddFreeSpace(patchSpaceStart, arm7PatchSpaceSize);
        }

        // The arm7 patcher uses the fact that the ntr wram is mirrored over the entire 03 region.
        // Since the reserved patch space is smaller than the ntr wram, it will never overlap.
        // As a result, the patcher can use arm7 addresses for placing patches.
        // The arm7 will copy the patch data to the actual arm7 location afterwards.

        // If in DSi mode and the rom is a DSi rom, temporarily disable twl wram to let ntr wram cover the entire 03 region.
        u32 mbk6 = 0;
        u32 mbk7 = 0;
        u32 mbk8 = 0;
        if (gIsDsiMode && romHeader->SupportsDsiMode())
        {
            mbk6 = REG_MBK6;
            mbk7 = REG_MBK7;
            mbk8 = REG_MBK8;
            REG_MBK6 = 0;
            REG_MBK7 = 0;
            REG_MBK8 = 0;
        }

        if (!patchCollection.TryPerformPatches(patchContext))
        {
            ErrorDisplay().PrintError("Failed to apply arm7 patches.");
        }
        dc_flushAll();
        dc_drainWriteBuffer();
        ic_invalidateAll();

        // If in DSi mode and the rom is a DSi rom, restore twl wram.
        if (gIsDsiMode && romHeader->SupportsDsiMode())
        {
            REG_MBK6 = mbk6;
            REG_MBK7 = mbk7;
            REG_MBK8 = mbk8;
        }

        mem_setNtrWramMapping(MEM_NTR_WRAM_ARM7, MEM_NTR_WRAM_ARM7);
    }
    else
    {
        LOG_DEBUG("Module params not found!\n");
    }
    return (u32)patchSpaceStart < 0x03000000 ? nullptr : patchSpaceStart;
}
