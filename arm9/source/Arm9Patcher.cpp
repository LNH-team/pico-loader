#include "common.h"
#include "ModuleParamsLocator.h"
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
#include "patches/arm9/NandSave/FaceTrainingNandSavePatch.h"
#include "patches/arm9/NandSave/JamWithTheBandNandSavePatch.h"
#include "patches/arm9/NandSave/NintendoDSGuideNandSavePatch.h"
#include "patches/arm9/NandSave/WarioWareDiyNandSavePatch.h"
#include "patches/arm9/OverlayPatches/FsStartOverlayHookPatch.h"
#include "patches/arm9/OverlayPatches/DSProtectPatches/DSProtectOverlayPatch.h"
#include "patches/arm9/OverlayPatches/DSProtectPatches/DSProtectPuyoPuyo7Patch.h"
#include "patches/arm9/OverlayPatches/PokemonIr/PokemonIrApPatch.h"
#include "patches/arm9/OverlayPatches/KirbySuperStarUltra/KirbySuperStarUltraPatch.h"
#include "patches/arm9/OverlayPatches/GoldenSunDarkDawn/GoldenSunDarkDawnOverlayHookPatch.h"
#include "SecureSysCallsUnusedSpaceLocator.h"
#include "fastSearch.h"
#include "gameCode.h"
#include "cache.h"
#include "ApList.h"
#include "patches/platform/LoaderPlatform.h"
#include "errorDisplay/ErrorDisplay.h"
#include "Arm9Patcher.h"

#define PARENT_SECTION_START    0x02001000
#define PARENT_SECTION_END      0x02003000

#define REQUIRED_PATCH_HEAP_SPACE   0x500

typedef void (*uncompress_func_t)(void* compressedEnd);

static const u32 sMiiUncompressBackwardPatternOld[] = { 0xE3500000, 0x0A000025, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPatternOld2[] = { 0xE3500000, 0x0A00002B, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPattern[] = { 0xE3500000, 0x0A000027, 0xE92D00F0, 0xE9100006 };
static const u32 sMiiUncompressBackwardPatternHybrid[] = { 0xE3500000, 0x0A000029, 0xE92D01F0, 0xE9100006 };

void Arm9Patcher::ApplyPatches(const LoaderPlatform* loaderPlatform, const ApListEntry* apListEntry,
    bool isCloneBootRom, const loader_info_t* loaderInfo) const
{
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    auto twlRomHeader = (const nds_header_twl_t*)TWL_SHARED_MEMORY->twlRomHeader;
    u32 arm9Size = romHeader->arm9Size;
    u32 arm9iSize = romHeader->IsTwlRom() ? twlRomHeader->arm9iSize : 0;
    u32 compressedEnd = 0;
    auto moduleParams = ModuleParamsLocator().FindModuleParams(romHeader);
    SdkVersion sdkVersion = moduleParams ? moduleParams->sdkVersion : 0u;
    if (moduleParams)
    {
        LOG_DEBUG("Module params found at 0x%p\n", moduleParams);
        LOG_DEBUG("Sdk version: 0x%x\n", moduleParams->sdkVersion);
        const u32* miiUncompressBackward = nullptr;
        if (moduleParams->compressedEnd)
        {
            miiUncompressBackward = FindMIiUncompressBackward(romHeader->arm9LoadAddress, sdkVersion);
            if (miiUncompressBackward)
            {
                arm9Size = moduleParams->compressedEnd + *(u32*)(moduleParams->compressedEnd - 4) - romHeader->arm9LoadAddress;
                ((uncompress_func_t)miiUncompressBackward)((void*)moduleParams->compressedEnd);
                compressedEnd = moduleParams->compressedEnd;
                moduleParams->compressedEnd = 0;
            }
            else
            {
                LOG_DEBUG("MIi_UncompressBackward not found\n");
            }
        }

        if (gIsDsiMode && romHeader->IsTwlRom())
        {
            auto arm9iModuleParams = (module_params_twl_t*)(romHeader->arm9LoadAddress + twlRomHeader->arm9iModuleParamsAddress);
            if (arm9iModuleParams->magicBigEndian == MODULE_PARAMS_TWL_MAGIC_BE &&
                arm9iModuleParams->magicLittleEndian == MODULE_PARAMS_TWL_MAGIC_LE &&
                arm9iModuleParams->compressedEnd != 0)
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
                    LOG_DEBUG("Could not decompress arm9i\n");
                }
            }
        }
    }
    else
    {
        LOG_DEBUG("Module params not found!\n");
        if (romHeader->gameCode == GAMECODE("AS2E"))
        {
            // Spider-Man 2 (USA) is probably the only game without module params
            sdkVersion = 0x02004F50;
        }
    }
    LOG_DEBUG("Arm9 region: 0x%x - 0x%x\n", romHeader->arm9LoadAddress, romHeader->arm9LoadAddress + arm9Size);
    PatchContext patchContext
    {
        (void*)romHeader->arm9LoadAddress,
        arm9Size,
        romHeader->IsTwlRom() ? (void*)twlRomHeader->arm9iLoadAddress : nullptr,
        arm9iSize,
        sdkVersion,
        romHeader->gameCode,
        loaderPlatform
    };
    PatchCollection patchCollection;
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
                // if ((romHeader->unitCode & 3) != 3)
                {
                    patchCollection.AddPatch(new CardiIsRomDmaAvailablePatch());
                }
                patchCollection.AddPatch(new CardiReadRomWithCpuPatch());

                if (gIsDsiMode && romHeader->IsTwlRom())
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
        patchCollection.AddPatch(new OSResetSystemPatch(loaderInfo));
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
    dc_flushAll();
    dc_drainWriteBuffer();
    ic_invalidateAll();
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
