#include "common.h"
#include "patches/PatchContext.h"
#include "fileInfo.h"
#include "thumbInstructions.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm9/RomOffsetToSdSectorPatchCode.h"
#include "patches/OffsetToSectorRemapPatchCode.h"
#include "patches/arm9/FixCp15Asm.h"
#include "gameCode.h"
#include "CardiReadCardPatchAsm.h"
#include "CardiReadCardPatch.h"
#include "patches/arm7/hotkey/RetailHotkeyDetectionPatchCode.h"
extern "C" u32 patch_cardireadcard_loaderParams[33];
extern "C" u32 patch_cardireadcard_loader9Extents[10];
extern "C" void patch_cardireadcard_retail_entry();
u32 retailReturnCardiMode = 0; // 1=return-capable, 2=compact boot fallback
u32 retailReturnTakeoverAddress = 0;
u32 retailReturnFixAddress = 0;
u32 retailReturnSplitPathWords = 32;
u32 retailReturnHeapBeforeTotal = 0;
u32 retailReturnHeapBeforeLargest = 0;

static const u32 sCARDiReadCardPatternUnknown[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE1A0A000u, 0xE59F90D8u };
static const u32 sCARDiReadCardPatternSdk20029A7[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE1A0A000u, 0xE59F90E0u };
static const u32 sCARDiReadCardPatternSdk2004E8BPingPals[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE1A0A000u, 0xE59F90FCu };
static const u32 sCARDiReadCardPatternSdk2004F4CThumb[] = { 0xB083B5F0u, 0x4E279000u, 0x30209001u, 0x4F269001u };
static const u32 sCARDiReadCardPatternSdk2007530MinnaNoMahjong[] = { 0xE92D47F0u, 0xE1A05000u, 0xE59F40D4u, 0xE5941018u };
static const u32 sCARDiReadCardPatternSdk2027533ThumbChouSoujuu[] = { 0xB083B5F0u, 0x4E259000u, 0x30209001u, 0x4F249001u };
static const u32 sCARDiReadCardPatternSdk3027530Thumb[] = { 0xB082B5F8u, 0x90019000u, 0x90013020u, 0x69C24825u };
static const u32 sCARDiReadCardPatternSdk4002774[] = { 0xE92D4070u, 0xE59F40D0u, 0xE1A06000u, 0xE3A00C02u };
static const u32 sCARDiReadCardPatternSdk4017530[] = { 0xE92D4070u, 0xE59F40D8u, 0xE1A06000u, 0xE3A00C02u };
static const u32 sCARDiReadCardPatternSdk4017530Thumb[] = { 0xB082B5F8u, 0x90019000u, 0x90013020u, 0x69C24827u };
static const u32 sCARDiReadCardPatternSdk4027531ThumbInlined[] = { 0xB082B5F8u, 0x90019000u, 0x90013020u, 0x69C14841u };
static const u32 sCARDiReadCardPatternSdk4027539SpiritTracks[] = { 0xE92D4070u, 0xE59F40D4u, 0xE1A06000u, 0xE3A00C02u };

void CardiReadCardPatch::TryPattern(PatchContext& patchContext, const u32* pattern, u32 byteLength)
{
    _cardiReadCard = patchContext.FindPattern32(pattern, byteLength);
    if (_cardiReadCard)
        _foundPattern = pattern;
}

bool CardiReadCardPatch::FindPatchTarget(PatchContext& patchContext)
{
    if (patchContext.GetGameCode() == GAMECODE("A4VJ"))
    {
        // Shaberu! DS Oryouri Navi (Japan) has a spurious match
        _foundPattern = sCARDiReadCardPatternUnknown;
        _thumb = false;
        _cardiReadCard = (u32*)(patchContext.GetGameRevision() == 0 ? 0x020DBD5C : 0x020DBD4C);
    }
    else
    {
        if (patchContext.GetSdkVersion() >= 0x4017530)
            TryPattern(patchContext, sCARDiReadCardPatternSdk4017530, sizeof(sCARDiReadCardPatternSdk4017530));
        else if (patchContext.GetSdkVersion() >= 0x4002774)
            TryPattern(patchContext, sCARDiReadCardPatternSdk4002774, sizeof(sCARDiReadCardPatternSdk4002774));
        else
            TryPattern(patchContext, sCARDiReadCardPatternSdk20029A7, sizeof(sCARDiReadCardPatternSdk20029A7));

        if (!_cardiReadCard)
        {
            if (patchContext.GetSdkVersion() >= 0x4017530)
                TryPattern(patchContext, sCARDiReadCardPatternSdk4017530Thumb, sizeof(sCARDiReadCardPatternSdk4017530Thumb));
            else if (patchContext.GetSdkVersion() >= 0x3027530)
                TryPattern(patchContext, sCARDiReadCardPatternSdk3027530Thumb, sizeof(sCARDiReadCardPatternSdk3027530Thumb));
            else if (patchContext.GetSdkVersion() >= 0x2004F4C)
                TryPattern(patchContext, sCARDiReadCardPatternSdk2004F4CThumb, sizeof(sCARDiReadCardPatternSdk2004F4CThumb));

            if (_cardiReadCard)
                _thumb = true;
        }

        if (!_cardiReadCard)
        {
            // if still nothing found try some of the patterns that appear all over the place
            TryPattern(patchContext, sCARDiReadCardPatternUnknown, sizeof(sCARDiReadCardPatternUnknown));
            if (!_cardiReadCard)
                TryPattern(patchContext, sCARDiReadCardPatternSdk4002774, sizeof(sCARDiReadCardPatternSdk4002774));
            if (!_cardiReadCard)
                TryPattern(patchContext, sCARDiReadCardPatternSdk20029A7, sizeof(sCARDiReadCardPatternSdk20029A7));
            if (patchContext.GetSdkVersion().GetMajor() >= SDK_VERSION_MAJOR_NITRO_4)
            {
                if (!_cardiReadCard)
                    TryPattern(patchContext, sCARDiReadCardPatternSdk4027539SpiritTracks, sizeof(sCARDiReadCardPatternSdk4027539SpiritTracks));
            }

            if (patchContext.GetSdkVersion().GetMajor() <= SDK_VERSION_MAJOR_NITRO_2)
            {
                if (!_cardiReadCard)
                    TryPattern(patchContext, sCARDiReadCardPatternSdk2007530MinnaNoMahjong, sizeof(sCARDiReadCardPatternSdk2007530MinnaNoMahjong));
                if (!_cardiReadCard)
                    TryPattern(patchContext, sCARDiReadCardPatternSdk2004E8BPingPals, sizeof(sCARDiReadCardPatternSdk2004E8BPingPals));
            }
        }

        if (!_cardiReadCard)
        {
            TryPattern(patchContext, sCARDiReadCardPatternSdk2004F4CThumb, sizeof(sCARDiReadCardPatternSdk2004F4CThumb));
            if (!_cardiReadCard)
                TryPattern(patchContext, sCARDiReadCardPatternSdk3027530Thumb, sizeof(sCARDiReadCardPatternSdk3027530Thumb));
            if (!_cardiReadCard)
                TryPattern(patchContext, sCARDiReadCardPatternSdk2027533ThumbChouSoujuu, sizeof(sCARDiReadCardPatternSdk2027533ThumbChouSoujuu));
            if (!_cardiReadCard)
                TryPattern(patchContext, sCARDiReadCardPatternSdk4027531ThumbInlined, sizeof(sCARDiReadCardPatternSdk4027531ThumbInlined));

            if (_cardiReadCard)
                _thumb = true;
        }
    }

    if (_cardiReadCard)
    {
        LOG_DEBUG("Found CARDi_ReadCard at %p\n", _cardiReadCard);
        // patchContext.GetPatchHeap().AddFreeSpace((u8*)_cardiReadCard + 0x64, 0x34);
    }
    else
    {
        LOG_DEBUG("CARDi_ReadCard not found\n");
    }
    return _cardiReadCard != nullptr;
}

void CardiReadCardPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiReadCard)
        return;

    // patch at CARDi_ReadCard + 0x58
    // r1 = rom src (512 byte aligned)
    // r8 = dst (32 bit aligned)
    // return to CARDi_ReadCard + 0x98

    retailReturnHeapBeforeTotal = patchContext.GetPatchHeap().GetFreeBytes();
    retailReturnHeapBeforeLargest = patchContext.GetPatchHeap().GetLargestFreeBlock();
    u32 patch1Size = SECTION_SIZE(patch_cardireadcard);
    void* patch1Address = patchContext.GetPatchHeap().TryAlloc(patch1Size);
    void* pieces[7] = {};
    const u32 pieceSizes[7] = {
        SECTION_SIZE(patch_cardireadcard_split), SECTION_SIZE(patch_split_ack),
        SECTION_SIZE(patch_split_park), SECTION_SIZE(patch_split_load),
        SECTION_SIZE(patch_split_check), 68, 40
    };
    // The split path accepts only complete paths fitting its 64-byte buffer.
    bool shortPath = false;
    for (u32 i = 0; i < 64; ++i)
        if (((const char*)&patch_cardireadcard_loaderParams[1])[i] == 0)
        { shortPath = i != 0; break; }
    const bool split = !patch1Address && shortPath &&
        patchContext.GetPatchHeap().TryAllocPieces(pieceSizes, pieces, 7);
    const bool full = patch1Address != nullptr;
    const void* patchTemplate = full ? SECTION_START(patch_cardireadcard) :
        split ? SECTION_START(patch_cardireadcard_split) : SECTION_START(patch_cardireadcard_compat);
    const u32 selectedEntry = full ? (u32)&patch_cardireadcard_entry :
        split ? (u32)&patch_cardireadcard_split_entry : (u32)&patch_cardireadcard_compat_entry;
    if (split)
    {
        patch1Size = pieceSizes[0];
        patch1Address = pieces[0];
    }
    else if (!full)
    {
        patch1Size = SECTION_SIZE(patch_cardireadcard_compat);
        patch1Address = patchContext.GetPatchHeap().Alloc(patch1Size);
    }
    retailReturnCardiMode = split ? 3 : full ? 1 : 2; // Test130: same split layout, stock epilogue, no detector or extra DLDI reservation
    retailReturnSplitPathWords = split ? 16 : 32;
    // The same SDK adaptations patch fields in whichever template is selected.
    u16& patch_cardireadcard_return_offset = full ?
        ::patch_cardireadcard_return_offset : split ? ::patch_cardireadcard_split_return_offset : ::patch_cardireadcard_compat_return_offset;
    u16& patch_cardireadcard_mov_src_to_r0 = full ?
        ::patch_cardireadcard_mov_src_to_r0 : split ? ::patch_cardireadcard_split_mov_src_to_r0 : ::patch_cardireadcard_compat_mov_src_to_r0;
    u16& patch_cardireadcard_mov_dst_to_r1 = full ?
        ::patch_cardireadcard_mov_dst_to_r1 : split ? ::patch_cardireadcard_split_mov_dst_to_r1 : ::patch_cardireadcard_compat_mov_dst_to_r1;
    u16& patch_cardireadcard_mov_cardicommon_to_r6 = full ?
        ::patch_cardireadcard_mov_cardicommon_to_r6 : split ? ::patch_cardireadcard_split_mov_cardicommon_to_r6 : ::patch_cardireadcard_compat_mov_cardicommon_to_r6;
    u16& patch_cardireadcard_adjust_cardicommon_offset = full ?
        ::patch_cardireadcard_adjust_cardicommon_offset : split ? ::patch_cardireadcard_split_adjust_cardicommon_offset : ::patch_cardireadcard_compat_adjust_cardicommon_offset;
    u16& patch_cardireadcard_mov_r3_to_dst = full ?
        ::patch_cardireadcard_mov_r3_to_dst : split ? ::patch_cardireadcard_split_mov_r3_to_dst : ::patch_cardireadcard_compat_mov_r3_to_dst;
    u32& __patch_cardireadcard_fix_cp15_asm_address = full ?
        ::__patch_cardireadcard_fix_cp15_asm_address : split ? ::__patch_cardireadcard_split_fix_cp15_asm_address : ::__patch_cardireadcard_compat_fix_cp15_asm_address;
    u32& __patch_cardireadcard_rom_offset_to_sd_sector_asm_address = full ?
        ::__patch_cardireadcard_rom_offset_to_sd_sector_asm_address : split ? ::__patch_cardireadcard_split_rom_offset_to_sd_sector_asm_address : ::__patch_cardireadcard_compat_rom_offset_to_sd_sector_asm_address;
    u32& __patch_cardireadcard_sdread_asm_address = full ?
        ::__patch_cardireadcard_sdread_asm_address : split ? ::__patch_cardireadcard_split_sdread_asm_address : ::__patch_cardireadcard_compat_sdread_asm_address;
    auto loaderPlatform = patchContext.GetLoaderPlatform();
    if (loaderPlatform->HasRomReads())
    {
        auto romReadPatchCode = loaderPlatform->CreateRomReadPatchCode(
            patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
        auto offsetToSectorRemapPatchCode = patchContext.GetPatchCodeCollection().GetOrAddSharedPatchCode([&]
        {
            return new OffsetToSectorRemapPatchCode(patchContext.GetPatchHeap());
        });
        __patch_cardireadcard_rom_offset_to_sd_sector_asm_address = (u32)offsetToSectorRemapPatchCode->GetRemapFunction();
        __patch_cardireadcard_sdread_asm_address = (u32)romReadPatchCode->GetReadSectorsFunction();
    }
    else
    {
        auto sdReadPatchCode = loaderPlatform->CreateSdReadPatchCode(
            patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
        auto romOffsetToSdSectorPatchCode = patchContext.GetPatchCodeCollection().GetOrAddSharedPatchCode([&]
        {
            return new RomOffsetToSdSectorPatchCode(patchContext.GetPatchHeap(),
                (const rom_file_info_t*)((u32)SHARED_ROM_FILE_INFO - 0x02F00000 + 0x02700000));
        });
        __patch_cardireadcard_rom_offset_to_sd_sector_asm_address = (u32)romOffsetToSdSectorPatchCode->GetRemapFunction();
        __patch_cardireadcard_sdread_asm_address = (u32)sdReadPatchCode->GetReadSectorsFunction();
    }
    u32 patch4Size = SECTION_SIZE(fixcp15);
    void* patch4Address = patchContext.GetPatchHeap().Alloc(patch4Size);
    __patch_cardireadcard_fix_cp15_asm_address = (u32)&fix_cp15_asm - (u32)SECTION_START(fixcp15) + (u32)patch4Address;

    u32 entryAddress = selectedEntry - (u32)patchTemplate + (u32)patch1Address;

    if (_thumb)
    {
        u32 patchOffset;
        if (_foundPattern == sCARDiReadCardPatternSdk4017530Thumb)
        {
            patchOffset = 0x36;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x3C);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R4); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_LDR_SP_IMM(THUMB_R6, 8); // r3 from the stack
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_STR_SP_IMM(THUMB_R3, 0xC); // copy to r4 on the stack
            *(u16*)((u8*)_cardiReadCard + patchOffset + 0) = THUMB_LDR_PC_IMM(THUMB_R3, 0x74); // ldr r3,= cardi_common
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk3027530Thumb)
        {
            patchOffset = 0x36;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x36);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R4); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_LDR_SP_IMM(THUMB_R6, 8); // r3 from the stack
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_STR_SP_IMM(THUMB_R3, 0xC); // copy to r4 on the stack
            *(u16*)((u8*)_cardiReadCard + patchOffset + 0) = THUMB_LDR_PC_IMM(THUMB_R3, 0x6C); // ldr r3,= cardi_common
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk2004F4CThumb || _foundPattern == sCARDiReadCardPatternSdk2027533ThumbChouSoujuu)
        {
            patchOffset = 0x36;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x2E);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R5); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_LDR_SP_IMM(THUMB_R6, 0x10); // r6 from stack
            if (patchContext.GetSdkVersion().GetMajor() <= SDK_VERSION_MAJOR_NITRO_2)
                patch_cardireadcard_adjust_cardicommon_offset = THUMB_SUBS_IMM(THUMB_R6, THUMB_R6, 4);
            else
                patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOVS_REG(THUMB_R5, THUMB_R3);
            *(u16*)((u8*)_cardiReadCard + patchOffset + 0) = THUMB_NOP;
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk4027531ThumbInlined)
        {
            // This variant has a lot of inlining
            patchOffset = 0x36;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x64);
            patch_cardireadcard_mov_src_to_r0 = THUMB_LDR_SP_IMM(THUMB_R0, 8); // src, r3 from the stack
            patch_cardireadcard_mov_dst_to_r1 = THUMB_LDR_SP_IMM(THUMB_R1, 4); // dst, r2 from the stack
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_LDR_SP_IMM(THUMB_R6, 0); // r1 from the stack
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_STR_SP_IMM(THUMB_R3, 4); // copy to r2 on the stack
            *(u16*)((u8*)_cardiReadCard + patchOffset + 0) = THUMB_LDR_PC_IMM(THUMB_R1, 0xDC); // ldr r1,= cardi_common
        }
        else
        {
            LOG_FATAL("Unsupported CARDi_ReadCard pattern\n");
            while (1);
        }

        *(u16*)((u8*)_cardiReadCard + patchOffset + 2) = THUMB_LDR_PC_IMM(THUMB_R0, 0); // ldr r0,= entryAddress
        *(u16*)((u8*)_cardiReadCard + patchOffset + 4) = THUMB_BLX(THUMB_R0);
        *(u32*)((u8*)_cardiReadCard + patchOffset + 6) = entryAddress;
    }
    else
    {
        u32 patchOffset;
        if (_foundPattern == sCARDiReadCardPatternSdk4027539SpiritTracks)
        {
            patchOffset = 0x44;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x44);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R2); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R5); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOVS_REG(THUMB_R6, THUMB_R4);
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOVS_REG(THUMB_R5, THUMB_R3);
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk4017530)
        {
            patchOffset = 0x44;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x48);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R2); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R5); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOVS_REG(THUMB_R6, THUMB_R4);
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOVS_REG(THUMB_R5, THUMB_R3);
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk4002774)
        {
            patchOffset = 0x44;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x40);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R2); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R5); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOVS_REG(THUMB_R6, THUMB_R4);
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOVS_REG(THUMB_R5, THUMB_R3);
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk20029A7 || _foundPattern == sCARDiReadCardPatternUnknown)
        {
            patchOffset = 0x58;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x38);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOV_HIREG(THUMB_R1, THUMB_HI_R8); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOV_HIREG(THUMB_R6, THUMB_HI_R9);
            if (patchContext.GetSdkVersion().GetMajor() <= SDK_VERSION_MAJOR_NITRO_2)
                patch_cardireadcard_adjust_cardicommon_offset = THUMB_SUBS_IMM(THUMB_R6, THUMB_R6, 4);
            else
                patch_cardireadcard_adjust_cardicommon_offset = THUMB_NOP;
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOV_HIREG(THUMB_HI_R8, THUMB_R3);
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk2004E8BPingPals)
        {
            patchOffset = 0x5C;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x40);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOV_HIREG(THUMB_R1, THUMB_HI_R8); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOV_HIREG(THUMB_R6, THUMB_HI_R9);
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_SUBS_IMM(THUMB_R6, THUMB_R6, 4);
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOV_HIREG(THUMB_HI_R8, THUMB_R3);
        }
        else if (_foundPattern == sCARDiReadCardPatternSdk2007530MinnaNoMahjong)
        {
            patchOffset = 0x54;
            patch_cardireadcard_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x50);
            patch_cardireadcard_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
            patch_cardireadcard_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R7); // dst
            patch_cardireadcard_mov_cardicommon_to_r6 = THUMB_MOVS_REG(THUMB_R6, THUMB_R4);
            patch_cardireadcard_adjust_cardicommon_offset = THUMB_SUBS_IMM(THUMB_R6, THUMB_R6, 4);
            patch_cardireadcard_mov_r3_to_dst = THUMB_MOVS_REG(THUMB_R7, THUMB_R3);
        }
        else
        {
            LOG_FATAL("Unsupported CARDi_ReadCard pattern\n");
            while (1);
        }

        *(u32*)((u8*)_cardiReadCard + patchOffset + 0) = 0xE59F0000; // ldr r0,= entryAddress
        *(u32*)((u8*)_cardiReadCard + patchOffset + 4) = 0xE12FFF30; // blx r0
        *(u32*)((u8*)_cardiReadCard + patchOffset + 8) = entryAddress;
    }

    // Never arm ARM7 or install idle hooks against the compact read-only template.
    patch_retailhotkeydetect_loaderParamsAddress = 0;
    retailReturnTakeoverAddress = retailReturnFixAddress = 0;
    if (full)
    {
        patch_retailhotkeydetect_loaderParamsAddress =
            (u32)patch1Address + (u32)patch_cardireadcard_loaderParams -
            (u32)SECTION_START(patch_cardireadcard);
        retailReturnTakeoverAddress = ((u32)patch1Address +
            ((u32)patch_cardireadcard_retail_entry & ~1u) -
            (u32)SECTION_START(patch_cardireadcard)) | 1u;
        retailReturnFixAddress = __patch_cardireadcard_fix_cp15_asm_address;
    }
    if (split)
    {
        auto address = [](void* base, void* source, const void* symbol) -> u32
        { return (u32)base + ((u32)symbol & ~1u) - (u32)source; };
        splitReadNext = address(pieces[1], SECTION_START(patch_split_ack), (void*)splitAckEntry) | 1u;
        splitAckNext = address(pieces[2], SECTION_START(patch_split_park), (void*)splitParkEntry) | 1u;
        splitParkLoad = address(pieces[3], SECTION_START(patch_split_load), (void*)splitLoadEntry) | 1u;
        splitLoadCheck = address(pieces[4], SECTION_START(patch_split_check), (void*)splitCheckEntry) | 1u;
        splitLoadExtents = (u32)pieces[6];
        splitLoadRead = __patch_cardireadcard_sdread_asm_address;
        memcpy(pieces[1], SECTION_START(patch_split_ack), pieceSizes[1]);
        memcpy(pieces[2], SECTION_START(patch_split_park), pieceSizes[2]);
        memcpy(pieces[3], SECTION_START(patch_split_load), pieceSizes[3]);
        memcpy(pieces[4], SECTION_START(patch_split_check), pieceSizes[4]);
        memcpy(pieces[5], patch_cardireadcard_loaderParams, 68);
        memcpy(pieces[6], patch_cardireadcard_loader9Extents, 40);
        patch_retailhotkeydetect_loaderParamsAddress = (u32)pieces[5];
        // Test132: publish the proven split takeover for the optional idle helper.
        retailReturnTakeoverAddress = splitReadNext;
        retailReturnFixAddress = __patch_cardireadcard_fix_cp15_asm_address;
    }
    memcpy(patch1Address, patchTemplate, patch1Size);
    memcpy(patch4Address, SECTION_START(fixcp15), patch4Size);
}
