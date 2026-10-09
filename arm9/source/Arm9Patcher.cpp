#include "common.h"
#include "sections.h"
extern u32 retailReturnHeapBeforeTotal;
extern u32 retailReturnHeapBeforeLargest;
#include "ModuleParamsLocator.h"
#include "AutoloadAdjuster.h"
#include "SdkVersion.h"
#include "patches/PatchCollection.h"
#include "patches/PatchContext.h"
#include "sharedMemory.h"
#include "patches/arm9/sdk2to4/CardiReadCardPatch.h"
#include "patches/arm9/sdk2to4/CardiTryReadCardDmaPatch.h"
#include "patches/arm9/sdk5/CardiIsRomDmaAvailablePatch.h"
#include "patches/arm9/sdk5/CardiReadCardWithHashInternalAsyncPatch.h"
#include "patches/arm9/sdk5/CardiReadRomWithCpuPatch.h"
#include "patches/arm9/CardiReadRomIdCorePatch.h"
#include "patches/arm9/OSResetSystemPatch.h"
#include "patches/arm9/PokemonDownloaderArm9Patch.h"
#include "patches/arm9/DSProtectArm9Patch.h"
#include "patches/arm9/LastWindowCrcPatch.h"
#include "patches/arm9/NandSave/FaceTrainingNandSavePatch.h"
#include "patches/arm9/NandSave/JamWithTheBandNandSavePatch.h"
#include "patches/arm9/NandSave/NintendoDSGuideNandSavePatch.h"
#include "patches/arm9/NandSave/WarioWareDiyNandSavePatch.h"
#include "patches/arm9/OverlayPatches/FsStartOverlayHookPatch.h"
#include "patches/arm9/OverlayPatches/DSProtectPatches/DSProtectOverlayPatch.h"
#include "patches/arm9/OverlayPatches/DSProtectPatches/DSProtectPuyoPuyo7Patch.h"
#include "patches/arm9/OverlayPatches/PokemonIr/PokemonIrApPatch.h"
#include "patches/arm9/OverlayPatches/KirbySuperStarUltra/KirbySuperStarUltraPatch.h"
#include "patches/arm9/OverlayPatches/RabbidsGoHome/RabbidsGoHomePatch.h"
#include "patches/arm9/OverlayPatches/GoldenSunDarkDawn/GoldenSunDarkDawnOverlayHookPatch.h"
#include "SecureSysCallsUnusedSpaceLocator.h"
#include "fastSearch.h"
#include "gameCode.h"
#include "cache.h"
#include "ApList.h"
#include "patches/platform/LoaderPlatform.h"
#include "errorDisplay/ErrorDisplay.h"
#include "Arm9Patcher.h"


DEFINE_SECTION_SYMBOLS(patch_retailreturnwait);
DEFINE_SECTION_SYMBOLS(patch_waitsplit_probe);
DEFINE_SECTION_SYMBOLS(patch_waitsplit_resume);
extern u32 retailReturnCardiMode;
extern "C" void waitSplitProbeEntry();
extern "C" void waitSplitRequestEntry();
extern "C" void waitSplitReturnEntry();
extern "C" u32 waitSplitRequestAddress;
extern "C" u32 waitSplitReturnAddress;
extern "C" u32 waitSplitFix;
extern "C" u32 waitSplitTakeover;

extern "C" void patch_retailreturnwait_entry();
extern "C" u32 patch_retailreturnwait_fix;
extern "C" u32 patch_retailreturnwait_takeover;
extern u32 retailReturnTakeoverAddress;
extern u32 retailReturnFixAddress;
u32 retailReturnWaitHookCount = 0;
u32 retailReturnArmWaitHookCount = 0;
u32 retailReturnAutoloadWaitHookCount = 0;
u32 retailReturnHaltSiteCount = 0;
u32 retailReturnHaltHookCount = 0;
u32 retailReturnWaitSite = 0;
u32 retailReturnWaitStub = 0;
u32 retailReturnWaitState = 0;
u32 retailReturnNearbyTotal = 0;
u32 retailReturnNearbyMax = 0;

static void installRetailWaitHooks(PatchContext& context, const nds_header_ntr_t* header, bool patchesInParent, u32 imageBytes)
{
    retailReturnWaitState = 1;
    retailReturnNearbyTotal = retailReturnNearbyMax = 0;
    retailReturnWaitHookCount = 0;
    retailReturnArmWaitHookCount = 0;
    retailReturnAutoloadWaitHookCount = 0;
    retailReturnHaltSiteCount = retailReturnHaltHookCount = 0;
    retailReturnWaitSite = retailReturnWaitStub = 0;
    if (!retailReturnTakeoverAddress || !retailReturnFixAddress || header->arm9Size < 0x800)
        return;
    u16* sites[4] = {};
    u32 count = 0;
    auto secure = (u16*)header->arm9LoadAddress;
    for (u32 i = 0; i + 1 < 0x800 / 2 && count < 4; ++i)
    {
        u16 svc = secure[i];
        if (secure[i + 1] != 0x4770)
            continue;
        if (svc != 0xDF03 && svc != 0xDF04 && svc != 0xDF05 && svc != 0xDF06)
            continue;
        if ((svc == 0xDF04 || svc == 0xDF05) && (i == 0 || secure[i - 1] != 0x2200))
            continue;
        sites[count++] = &secure[i];
    }
    // Autoload bytes are patched in place, but branches use their final PC.
    u32* armSites[8] = {};
    u32 armCount = 0;
    u32 armRuntimeSites[8] = {};
    const auto adjuster = context.GetAutoloadAdjuster();
    auto words = (u32*)header->arm9LoadAddress;
    for (u32 i = 0x800 / 4; i + 1 < imageBytes / 4 && armCount < 8; ++i)
    {
        const u32 svc = words[i];
        if (words[i + 1] != 0xE12FFF1E)
            continue;
        if (svc != 0xEF030000 && svc != 0xEF040000 &&
            svc != 0xEF050000 && svc != 0xEF060000)
            continue;
        if ((svc == 0xEF040000 || svc == 0xEF050000) && words[i - 1] != 0xE3A02000)
            continue;
        const u32 initial = (u32)&words[i];
        const u32 runtime = adjuster ? adjuster->AdjustInitialToFinal(initial) : initial;
        // Never publish across an autoload boundary or into an unmapped region.
        if ((runtime & 3) || (adjuster &&
            adjuster->AdjustInitialToFinal(initial + 4) != runtime + 4))
            continue;
        if ((svc == 0xEF040000 || svc == 0xEF050000) && adjuster &&
            adjuster->AdjustInitialToFinal(initial - 4) != runtime - 4)
            continue;
        if (runtime != initial &&
            !((runtime >= 0x01FF8000 && runtime < 0x02000000) ||
              (runtime >= 0x02003000 && runtime < 0x02400000)))
            continue;
        armRuntimeSites[armCount] = runtime;
        armSites[armCount++] = &words[i];
    }
    // CP15 wait-for-interrupt can be inline, without any BIOS SVC wrapper.
    u32* haltSites[4] = {};
    u32 haltRuntimeSites[4] = {};
    u32 haltCount = 0;
    for (u32 i = 0x800 / 4; i + 1 < imageBytes / 4; ++i)
    {
        const u32 instruction = words[i];
        // Unconditional MCR p15,0,Rd,c7,c0,4; never accept Rd=PC.
        if ((instruction & 0xFFFF0FFF) != 0xEE070F90 ||
            (instruction & 0xF000) == 0xF000)
            continue;
        ++retailReturnHaltSiteCount;
        if (haltCount == 4)
            continue;
        const u32 initial = (u32)&words[i];
        const u32 runtime = adjuster ? adjuster->AdjustInitialToFinal(initial) : initial;
        if ((runtime & 3) || (adjuster &&
            adjuster->AdjustInitialToFinal(initial + 4) != runtime + 4))
            continue;
        if (runtime != initial &&
            !((runtime >= 0x01FF8000 && runtime + 4 < 0x02000000) ||
              (runtime >= 0x02003000 && runtime + 4 < 0x02400000)))
            continue;
        haltRuntimeSites[haltCount] = runtime;
        haltSites[haltCount++] = &words[i];
    }
    retailReturnWaitState = 2;
    if (!count && !armCount && !haltCount)
        return;
    const u32 codeSize = (SECTION_SIZE(patch_retailreturnwait) + 3) & ~3u;
    PatchHeap nearbyHeap;
    PatchHeap* stubHeap = &context.GetPatchHeap();
    if (patchesInParent)
    {
        retailReturnWaitState = 3;
        const u32 secureStart = header->arm9LoadAddress;
        if (!((u64)secureStart + 0x800 <= 0x02001000 || secureStart >= 0x02003000))
            return;
        SecureSysCallsUnusedSpaceLocator().FindUnusedSpace(header, nearbyHeap);
        stubHeap = &nearbyHeap;
    }
    retailReturnNearbyTotal = stubHeap->GetFreeBytes();
    retailReturnNearbyMax = stubHeap->GetLargestFreeBlock();
    retailReturnWaitState = 4;
    if (retailReturnNearbyTotal == 0)
    {
        // If the secure locator rejects its signature, show the two words.
        retailReturnWaitSite = *(const u32*)header->arm9LoadAddress;
        retailReturnWaitStub = *((const u32*)header->arm9LoadAddress + 1);
        return;
    }
    // Keep ARM checker in the proven heap; only small Thumb stubs need proximity.
    retailReturnWaitState = 5;
    u32 entry = 0;
    if (retailReturnCardiMode == 3)
    {
        const u32 sizes[] = { SECTION_SIZE(patch_waitsplit_probe), SECTION_SIZE(patch_waitsplit_resume) };
        void* pieces[2] = {};
        if (!context.GetPatchHeap().TryAllocPieces(sizes, pieces, 2))
            return;
        waitSplitRequestAddress = (u32)pieces[1] + (u32)waitSplitRequestEntry -
            (u32)SECTION_START(patch_waitsplit_resume);
        waitSplitReturnAddress = (u32)pieces[1] + (u32)waitSplitReturnEntry -
            (u32)SECTION_START(patch_waitsplit_resume);
        waitSplitFix = retailReturnFixAddress;
        waitSplitTakeover = retailReturnTakeoverAddress;
        memcpy(pieces[0], SECTION_START(patch_waitsplit_probe), sizes[0]);
        memcpy(pieces[1], SECTION_START(patch_waitsplit_resume), sizes[1]);
        entry = (u32)pieces[0] + (u32)waitSplitProbeEntry -
            (u32)SECTION_START(patch_waitsplit_probe);
    }
    else
    {
        // Preserve the hardware-passing Mario Kart helper and layout.
        auto allocation = (u8*)context.GetPatchHeap().TryAlloc(codeSize + 4);
        if (!allocation)
            return;
        auto block = (u8*)(((u32)allocation + 3) & ~3u);
        patch_retailreturnwait_fix = retailReturnFixAddress;
        patch_retailreturnwait_takeover = retailReturnTakeoverAddress;
        memcpy(block, SECTION_START(patch_retailreturnwait), SECTION_SIZE(patch_retailreturnwait));
        entry = (u32)block + (u32)patch_retailreturnwait_entry -
            (u32)SECTION_START(patch_retailreturnwait);
    }
    u16* stubs[4] = {};
    retailReturnWaitState = 6;
    for (u32 i = 0; i < count; ++i)
    {
        auto stubAllocation = (u8*)stubHeap->TryAlloc(20);
        if (!stubAllocation)
            continue;
        auto stub = (u16*)(((u32)stubAllocation + 3) & ~3u);
        int delta = (int)stub - (int)sites[i] - 4;
        retailReturnWaitSite = (u32)sites[i];
        retailReturnWaitStub = (u32)stub;
        if ((delta & 1) || delta < -2048 || delta > 2046)
            continue;
        stub[0] = 0xB50F;
        stub[1] = 0x4B02;
        stub[2] = 0x4798;
        stub[3] = 0xBC0F;
        stub[4] = *sites[i];
        stub[5] = 0xBD00;
        *(u32*)&stub[6] = entry;
        stubs[i] = stub;
    }
    // Prepare every accepted stub before publishing any hook.
    for (u32 i = 0; i < count; ++i)
    {
        if (!stubs[i])
            continue;
        int delta = (int)stubs[i] - (int)sites[i] - 4;
        *sites[i] = 0xE000 | ((delta >> 1) & 0x7FF);
        ++retailReturnWaitHookCount;
    }
    // ARM branch preserves LR, so original caller survives the detour.
    // Reuse the same checker; allocate no second helper.
    u32* armStubs[8] = {};
    for (u32 i = 0; i < armCount; ++i)
    {
        auto allocation = (u8*)stubHeap->TryAlloc(32);
        if (!allocation)
            continue;
        auto stub = (u32*)(((u32)allocation + 3) & ~3u);
        int delta = (int)stub - (int)armRuntimeSites[i] - 8;
        if ((delta & 3) || delta < -33554432 || delta > 33554428)
            continue;
        stub[0] = 0xE92D500F; // save BIOS arguments, r12, caller LR
        stub[1] = 0xE59FC00C; // helper literal at stub+24
        stub[2] = 0xE12FFF3C; // blx r12
        stub[3] = 0xE8BD500F;
        stub[4] = *armSites[i]; // original BIOS call
        stub[5] = 0xE12FFF1E; // bx original LR
        stub[6] = entry;
        armStubs[i] = stub;
    }
    for (u32 i = 0; i < armCount; ++i)
    {
        if (!armStubs[i])
            continue;
        int delta = (int)armStubs[i] - (int)armRuntimeSites[i] - 8;
        *armSites[i] = 0xEA000000 | ((delta >> 2) & 0xFFFFFF);
        ++retailReturnArmWaitHookCount;
        if (armRuntimeSites[i] != (u32)armSites[i])
            ++retailReturnAutoloadWaitHookCount;
    }
    // Inline halt must resume at the displaced instruction's successor,
    // not BX LR: the caller may be in the middle of a larger function.
    u32* haltStubs[4] = {};
    for (u32 i = 0; i < haltCount; ++i)
    {
        auto allocation = (u8*)stubHeap->TryAlloc(36);
        if (!allocation)
            continue;
        auto stub = (u32*)(((u32)allocation + 3) & ~3u);
        const int delta = (int)stub - (int)haltRuntimeSites[i] - 8;
        if ((delta & 3) || delta < -33554432 || delta > 33554428)
            continue;
        stub[0] = 0xE92D500F;
        stub[1] = 0xE59FC00C; // helper literal at stub+24
        stub[2] = 0xE12FFF3C;
        stub[3] = 0xE8BD500F; // restore every saved register, including LR
        stub[4] = *haltSites[i]; // original CP15 halt, original operand
        stub[5] = 0xE59FF000; // absolute ARM continuation at stub+28
        stub[6] = entry;
        stub[7] = haltRuntimeSites[i] + 4;
        haltStubs[i] = stub;
    }
    for (u32 i = 0; i < haltCount; ++i)
    {
        if (!haltStubs[i])
            continue;
        const int delta = (int)haltStubs[i] - (int)haltRuntimeSites[i] - 8;
        *haltSites[i] = 0xEA000000 | ((delta >> 2) & 0xFFFFFF);
        ++retailReturnHaltHookCount;
    }
    retailReturnWaitState = (retailReturnWaitHookCount || retailReturnArmWaitHookCount ||
        retailReturnHaltHookCount) ? 8 : 7;

}

#define PARENT_SECTION_START    0x02001000
#define PARENT_SECTION_END      0x02003000

#define REQUIRED_PATCH_HEAP_SPACE   0x500

typedef void (*uncompress_func_t)(void* compressedEnd);

static const u32 sMiiUncompressBackwardPatternOld[] = { 0xE3500000, 0x0A000025, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPatternOld2[] = { 0xE3500000, 0x0A00002B, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPattern[] = { 0xE3500000, 0x0A000027, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPatternHybrid[] = { 0xE3500000, 0x0A000029, 0xE92D01F0, 0xE9100006 };

Arm9Patcher::PatchResult Arm9Patcher::ApplyPatches(const LoaderPlatform* loaderPlatform, const ApListEntry* apListEntry,
    bool isCloneBootRom, bool runInDSiMode, const loader_info_t* loaderInfo) const
{
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    auto twlRomHeader = (const nds_header_twl_t*)TWL_SHARED_MEMORY->twlRomHeader;
    u32 arm9Size = romHeader->arm9Size;
    u32 arm9iSize = romHeader->SupportsDsiMode() ? twlRomHeader->arm9iSize : 0;
    u32 compressedEnd = 0;
    std::unique_ptr<IAutoloadAdjuster> arm9Autoload;
    std::unique_ptr<IAutoloadAdjuster> arm9iAutoload;
    module_params_ntr_t* moduleParams = nullptr;
    SdkVersion sdkVersion = 0u;
    if (romHeader->gameCode == GAMECODE("AS2E"))
    {
        // Spider-Man 2 (USA) is probably the only game without module params
        sdkVersion = 0x02004F50;
        arm9Autoload = std::make_unique<AutoloadAdjuster<autoload_list_entry_t>>(
            (autoload_list_entry_t*)0x0215DBA0,
            (autoload_list_entry_t*)0x0215DBB8,
            0x02157CC0);
    }
    else
    {
        moduleParams = ModuleParamsLocator().FindModuleParams(romHeader);
        if (moduleParams)
        {
            LOG_DEBUG("Module params found at 0x%p\n", moduleParams);
            sdkVersion = moduleParams->sdkVersion;
            LOG_DEBUG("Sdk version: 0x%x\n", sdkVersion);
            const u32* miiUncompressBackward = nullptr;
            if (moduleParams->compressedEnd)
            {
                miiUncompressBackward = FindMIiUncompressBackward(romHeader->arm9LoadAddress, sdkVersion);
                if (miiUncompressBackward)
                {
                    u32 originalBottom = *(u32*)(moduleParams->compressedEnd - 4);
                    // Some rom hacks decompress the arm9, but don't set compressedEnd to 0.
                    if (originalBottom < 4 * 1024 * 1024)
                    {
                        arm9Size = moduleParams->compressedEnd + originalBottom - romHeader->arm9LoadAddress;
                        ((uncompress_func_t)miiUncompressBackward)((void*)moduleParams->compressedEnd);
                    }
                    else
                    {
                        LOG_WARNING("Invalid originalBottom value found. Skipping arm9 decompression.\n");
                    }
                    compressedEnd = moduleParams->compressedEnd;
                    moduleParams->compressedEnd = 0;
                }
                else
                {
                    LOG_ERROR("MIi_UncompressBackward not found\n");
                }
            }

            if (sdkVersion.IsTwlSdk())
            {
                arm9Autoload = std::make_unique<AutoloadAdjuster<autoload_list_entry_sdk5_t>>(
                    (autoload_list_entry_sdk5_t*)moduleParams->autoloadListStart,
                    (autoload_list_entry_sdk5_t*)moduleParams->autoloadListEnd,
                    moduleParams->autoloadStart);
            }
            else
            {
                arm9Autoload = std::make_unique<AutoloadAdjuster<autoload_list_entry_t>>(
                    (autoload_list_entry_t*)moduleParams->autoloadListStart,
                    (autoload_list_entry_t*)moduleParams->autoloadListEnd,
                    moduleParams->autoloadStart);
            }

            if (runInDSiMode)
            {
                auto arm9iModuleParams = (module_params_twl_t*)(romHeader->arm9LoadAddress + twlRomHeader->arm9iModuleParamsAddress);
                if (arm9iModuleParams->magicBigEndian == MODULE_PARAMS_TWL_MAGIC_BE &&
                    arm9iModuleParams->magicLittleEndian == MODULE_PARAMS_TWL_MAGIC_LE)
                {
                    if (arm9iModuleParams->compressedEnd != 0)
                    {
                        LOG_DEBUG("Compressed arm9i found\n");
                        if (miiUncompressBackward)
                        {
                            arm9iSize = arm9iModuleParams->compressedEnd + *(u32*)(arm9iModuleParams->compressedEnd - 4) - twlRomHeader->arm9iLoadAddress;
                            ((uncompress_func_t)miiUncompressBackward)((void*)arm9iModuleParams->compressedEnd);
                            arm9iModuleParams->compressedEnd = 0;
                            LOG_DEBUG("Decompressed arm9i\n");
                        }
                        else
                        {
                            LOG_ERROR("Could not decompress arm9i\n");
                        }
                    }
                    arm9iAutoload = std::make_unique<AutoloadAdjuster<autoload_list_entry_sdk5_t>>(
                        (autoload_list_entry_sdk5_t*)arm9iModuleParams->autoloadListStart,
                        (autoload_list_entry_sdk5_t*)arm9iModuleParams->autoloadListEnd,
                        arm9iModuleParams->autoloadStart);
                }
            }
        }
        else
        {
            LOG_ERROR("Module params not found!\n");
        }
    }
    LOG_DEBUG("Arm9 region: 0x%x - 0x%x\n", romHeader->arm9LoadAddress, romHeader->arm9LoadAddress + arm9Size);
    PatchContext patchContext
    {
        (void*)romHeader->arm9LoadAddress,
        arm9Size,
        std::move(arm9Autoload),
        romHeader->SupportsDsiMode() ? (void*)twlRomHeader->arm9iLoadAddress : nullptr,
        arm9iSize,
        std::move(arm9iAutoload),
        sdkVersion,
        romHeader->gameCode,
        romHeader->softwareVersion,
        loaderPlatform
    };
    PatchCollection patchCollection;
    OSResetSystemPatch* osResetSystemPatch = nullptr;
    bool patchesInParent = false;
    if (sdkVersion != 0)
    {
        if (*(vu32*)0x02FFF00C == GAMECODE("ADAJ") &&
            romHeader->arm9LoadAddress == 0x02004000 &&
            romHeader->arm9EntryAddress == 0x02004800)
        {
            // pokemon downloader
            patchContext.GetPatchHeap().AddFreeSpace((void*)0x023FF160, 0x6A0);
            patchCollection.AddPatch(new PokemonDownloaderArm9Patch(loaderInfo));
        }
        else
        {
            u32 availableParentSize = 0;
            if (isCloneBootRom)
            {
                availableParentSize = GetAvailableParentSectionSpace();
                LOG_DEBUG("0x%X bytes available in .parent section\n", availableParentSize);
            }

            if (availableParentSize >= REQUIRED_PATCH_HEAP_SPACE)
            {
                patchContext.GetPatchHeap().AddFreeSpace(
                    (void*)(PARENT_SECTION_END - REQUIRED_PATCH_HEAP_SPACE),
                    REQUIRED_PATCH_HEAP_SPACE);
                patchesInParent = true;
                LOG_DEBUG("Placing patches in .parent section\n");
            }
            else
            {
                SecureSysCallsUnusedSpaceLocator().FindUnusedSpace(romHeader, patchContext.GetPatchHeap());
            }
        }

        if (sdkVersion.IsTwlSdk())
        {
            if (!twlRomHeader->IsDsiWare())
            {
                patchCollection.AddPatch(new CardiIsRomDmaAvailablePatch());
                patchCollection.AddPatch(new CardiReadRomWithCpuPatch());

                if (runInDSiMode)
                {
                    patchCollection.AddPatch(new CardiReadCardWithHashInternalAsyncPatch());
                }
            }
        }
        else
        {
            patchCollection.AddPatch(new CardiReadCardPatch());
            patchCollection.AddPatch(new CardiTryReadCardDmaPatch());
        }

        patchCollection.AddPatch(new CardiReadRomIdCorePatch());
        osResetSystemPatch = new OSResetSystemPatch(loaderInfo, runInDSiMode);
        patchCollection.AddPatch(osResetSystemPatch);
        AddGamePatches(patchCollection, romHeader->gameCode, apListEntry);

        if (moduleParams && compressedEnd != 0)
        {
            AddRestoreCompressedEndPatch(
                patchContext,
                romHeader->arm9AutoLoadDoneHookAddress,
                &moduleParams->compressedEnd,
                compressedEnd);
        }
    }
    if (!patchCollection.TryPerformPatches(patchContext))
    {
        ErrorDisplay().PrintError("Failed to apply arm9 patches.");
    }
    if (sdkVersion != 0 && !sdkVersion.IsTwlSdk())
    {
        installRetailWaitHooks(patchContext, romHeader, patchesInParent, arm9Size);
        // Test133: successful launches proceed normally; allocation-failure diagnostics remain.
    }
    dc_flushAll();
    dc_drainWriteBuffer();
    ic_invalidateAll();

    void** softResetCheatsPointer = nullptr;
    if (osResetSystemPatch != nullptr)
    {
        softResetCheatsPointer = osResetSystemPatch->GetCheatsPointerAtTarget();
    }

    return PatchResult
    {
        .softResetCheatsPointer = softResetCheatsPointer
    };
}

const u32* Arm9Patcher::FindMIiUncompressBackward(u32 arm9LoadAddress, SdkVersion sdkVersion) const
{
    const u32* miiUncompressBackwardPattern;
    if (sdkVersion <= 0x2017532)
    {
        miiUncompressBackwardPattern = sMiiUncompressBackwardPatternOld;
    }
    else
    {
        miiUncompressBackwardPattern = sMiiUncompressBackwardPattern;
    }

    const u32* miiUncompressBackward = fastSearch16((const u32*)arm9LoadAddress, 0x1000, miiUncompressBackwardPattern);
    if (!sdkVersion.IsTwlSdk() && !miiUncompressBackward)
    {
        miiUncompressBackward = fastSearch16((const u32*)arm9LoadAddress, 0x1000, sMiiUncompressBackwardPatternOld2);
    }
    if (sdkVersion.IsTwlSdk() && !miiUncompressBackward)
    {
        miiUncompressBackward = fastSearch16((const u32*)arm9LoadAddress, 0x1000, sMiiUncompressBackwardPatternHybrid);
    }

    return miiUncompressBackward;
}

void Arm9Patcher::AddGamePatches(PatchCollection& patchCollection, u32 gameCode, const ApListEntry* apListEntry) const
{
    OverlayHookPatch* overlayHookPatch;
    if (gameCode == GAMECODE("BO5P") || gameCode == GAMECODE("BO5E") || gameCode == GAMECODE("BO5J"))
    {
        overlayHookPatch = new GoldenSunDarkDawnOverlayHookPatch();
        overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(334, 0, DSProtectVersion::v2_01, ~0u));
        overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(335, 0, DSProtectVersion::v2_01s, ~0u));
    }
    else
    {
        overlayHookPatch = new FsStartOverlayHookPatch();
        if (apListEntry)
        {
            AddDSProtectPatches(patchCollection, overlayHookPatch, apListEntry);
        }
        AddGameSpecificPatches(patchCollection, overlayHookPatch, gameCode);
    }

    patchCollection.AddPatch(overlayHookPatch);
}

void Arm9Patcher::AddDSProtectPatches(
    PatchCollection& patchCollection, OverlayHookPatch* overlayHookPatch, const ApListEntry* apListEntry) const
{
    u32 regularOverlayId = apListEntry->GetRegularOverlayId();
    if (regularOverlayId != AP_LIST_OVERLAY_ID_INVALID)
    {
        if (regularOverlayId == AP_LIST_OVERLAY_ID_STATIC_ARM9)
        {
            patchCollection.AddPatch(new DSProtectArm9Patch(
                apListEntry->GetRegularOffset(), apListEntry->GetDSProtectVersion(),
                apListEntry->GetDSProtectFunctionMask()));
        }
        else
        {
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(
                regularOverlayId, apListEntry->GetRegularOffset(),
                apListEntry->GetDSProtectVersion(), apListEntry->GetDSProtectFunctionMask()));
        }
    }
    u32 sOverlayId = apListEntry->GetSOverlayId();
    if (sOverlayId != AP_LIST_OVERLAY_ID_INVALID)
    {
        auto version = apListEntry->GetDSProtectVersion();
        if (version < DSProtectVersion::v2_00s)
        {
            version = (DSProtectVersion)((u32)version - (u32)DSProtectVersion::v2_00 + (u32)DSProtectVersion::v2_00s);
        }
        if (sOverlayId == AP_LIST_OVERLAY_ID_STATIC_ARM9)
        {
            patchCollection.AddPatch(new DSProtectArm9Patch(
                apListEntry->GetSOffset(), version, ~0u));
        }
        else
        {
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(
                sOverlayId, apListEntry->GetSOffset(), version, ~0u));
        }
    }
}

void Arm9Patcher::AddGameSpecificPatches(
    PatchCollection& patchCollection, OverlayHookPatch* overlayHookPatch, u32 gameCode) const
{
    switch (gameCode)
    {
        // Dragon Ball: Origins 2
        case GAMECODE("BDBE"):
        {
            // BDBE;2;1.23;111111;0x1FC;-1;0x0
            // BDBE;3;1.23;111111;0x47DC;-1;0x0
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(2, 0x1FC, DSProtectVersion::v1_23, ~0u));
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(3, 0x47DC, DSProtectVersion::v1_23, ~0u));
            break;
        }
        case GAMECODE("BDBJ"):
        {
            // BDBJ;2;1.23;111111;0x1FC;-1;0x0
            // BDBJ;3;1.23;111111;0x4C34;-1;0x0
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(2, 0x1FC, DSProtectVersion::v1_23, ~0u));
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(3, 0x4C34, DSProtectVersion::v1_23, ~0u));
            break;
        }
        case GAMECODE("BDBP"):
        {
            // BDBP;2;1.23;111111;0x1FC;-1;0x0
            // BDBP;3;1.23;111111;0x484C;-1;0x0
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(2, 0x1FC, DSProtectVersion::v1_23, ~0u));
            overlayHookPatch->AddOverlayPatch(new DSProtectOverlayPatch(3, 0x484C, DSProtectVersion::v1_23, ~0u));
            break;
        }
        // Puyo Puyo 7
        case GAMECODE("BYOJ"):
        {
            // BYOJ;9;1.08;100110;1.08;0x21AC;-1;0x0
            // BYOJ;12;1.08;100101;1.08;0xC568;-1;0x0
            // BYOJ;14;1.08;010101;1.08;0x13AB8;-1;0x0
            // BYOJ;15;1.08;010110;1.08;0x16DF0;-1;0x0
            // BYOJ;19;1.08;011010;1.08;0x17F8;-1;0x0
            overlayHookPatch->AddOverlayPatch(new DSProtectPuyoPuyo7Patch());
            break;
        }
        // Pokemon HeartGold & SoulSilver
        case GAMECODE("IPGD"):
        case GAMECODE("IPGE"):
        case GAMECODE("IPGF"):
        case GAMECODE("IPGI"):
        case GAMECODE("IPGJ"):
        case GAMECODE("IPGK"):
        case GAMECODE("IPGS"):
        case GAMECODE("IPKD"):
        case GAMECODE("IPKE"):
        case GAMECODE("IPKF"):
        case GAMECODE("IPKI"):
        case GAMECODE("IPKJ"):
        case GAMECODE("IPKK"):
        case GAMECODE("IPKS"):
        {
            overlayHookPatch->AddOverlayPatch(new PokemonIrApPatch(PokemonIrVersion::Hgss));
            break;
        }
        // Pokemon Black & White
        case GAMECODE("IRAD"):
        case GAMECODE("IRAF"):
        case GAMECODE("IRAI"):
        case GAMECODE("IRAJ"):
        case GAMECODE("IRAK"):
        case GAMECODE("IRAO"):
        case GAMECODE("IRAS"):
        case GAMECODE("IRBD"):
        case GAMECODE("IRBF"):
        case GAMECODE("IRBI"):
        case GAMECODE("IRBJ"):
        case GAMECODE("IRBK"):
        case GAMECODE("IRBO"):
        case GAMECODE("IRBS"):
        {
            overlayHookPatch->AddOverlayPatch(new PokemonIrApPatch(PokemonIrVersion::Bw1));
            break;
        }
        // Pokemon Black & White 2
        case GAMECODE("IRDD"):
        case GAMECODE("IRDF"):
        case GAMECODE("IRDI"):
        case GAMECODE("IRDJ"):
        case GAMECODE("IRDK"):
        case GAMECODE("IRDO"):
        case GAMECODE("IRDS"):
        case GAMECODE("IRED"):
        case GAMECODE("IREF"):
        case GAMECODE("IREI"):
        case GAMECODE("IREJ"):
        case GAMECODE("IREK"):
        case GAMECODE("IREO"):
        case GAMECODE("IRES"):
        {
            overlayHookPatch->AddOverlayPatch(new PokemonIrApPatch(PokemonIrVersion::Bw2));
            break;
        }
        // WarioWare: D.I.Y.
        case GAMECODE("UORE"):
        case GAMECODE("UORP"):
        case GAMECODE("UORJ"):
        {
            patchCollection.AddPatch(new WarioWareDiyNandSavePatch());
            break;
        }
        // Jam with the Band
        case GAMECODE("UXBP"):
        {
            patchCollection.AddPatch(new JamWithTheBandNandSavePatch());
            break;
        }
        // Face Training
        case GAMECODE("USKV"):
        {
            patchCollection.AddPatch(new FaceTrainingNandSavePatch());
            break;
        }
        // Nintendo DS Guide
        case GAMECODE("UGDA"):
        {
            patchCollection.AddPatch(new NintendoDSGuideNandSavePatch());
            break;
        }
        // Last Window: The Secret of Cape West
        case GAMECODE("YLUP"):
        {
            patchCollection.AddPatch(new LastWindowCrcPatch());
            break;
        }
        // Rabbids Go Home
        case GAMECODE("VRGE"):
        case GAMECODE("VRGV"):
        {
            overlayHookPatch->AddOverlayPatch(new RabbidsGoHomePatch());
            break;
        }
        // Kirby Super Star Ultra
        case GAMECODE("YKWE"):
        case GAMECODE("YKWJ"):
        case GAMECODE("YKWK"):
        case GAMECODE("YKWP"):
        {
            overlayHookPatch->AddOverlayPatch(new KirbySuperStarUltraPatch());
            break;
        }
    }
}

void Arm9Patcher::AddRestoreCompressedEndPatch(PatchContext& patchContext,
    u32 arm9AutoLoadDoneHookAddress, u32* moduleParamsCompressedEnd, u32 originalCompressedEndValue) const
{
    // Restore compressedEnd after first boot.
    // This is necessary to not break cloneboot.
    const u32 compressedEndFixCode[] =
    {
        0xE59F0014, // ldr r0,= moduleParamsCompressedEnd
        0xE59F1014, // ldr r1,= originalCompressedEndValue
        0xE5801000, // str r1, [r0]
        0xE59F0010, // ldr r0,= arm9AutoLoadDoneHookAddress
        0xE59F1000, // ldr r1, ret
        0xE5801000, // str r1, [r0]
        0xE12FFF1E, // ret: bx lr
        (u32)moduleParamsCompressedEnd,
        originalCompressedEndValue,
        arm9AutoLoadDoneHookAddress
    };

    void* fixDst = patchContext.GetPatchHeap().Alloc(sizeof(compressedEndFixCode));
    memcpy(fixDst, compressedEndFixCode, sizeof(compressedEndFixCode));
    *(u32*)arm9AutoLoadDoneHookAddress = 0xEA000000u | ((((int)fixDst - (int)arm9AutoLoadDoneHookAddress - 8) >> 2) & 0xFFFFFF);
}

u32 Arm9Patcher::GetAvailableParentSectionSpace() const
{
    u32 availableParentSize = 0;
    for (u32 ptr = PARENT_SECTION_END; ptr > PARENT_SECTION_START; ptr -= 32)
    {
        u32* segment = (u32*)(ptr - 32);
        if (segment[0] != 0 || segment[1] != 0 || segment[2] != 0 || segment[3] != 0 ||
            segment[4] != 0 || segment[5] != 0 || segment[6] != 0 || segment[7] != 0)
        {
            break;
        }

        availableParentSize += 32;
    }

    return availableParentSize;
}
