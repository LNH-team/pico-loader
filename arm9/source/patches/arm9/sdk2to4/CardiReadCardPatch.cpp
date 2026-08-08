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
#include "DashCmdPatchCode.h"

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

    u32 patch1Size = SECTION_SIZE(patch_cardireadcard);
    void* patch1Address = patchContext.GetPatchHeap().Alloc(patch1Size);
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
    // v19 (Pokemon Dash) : la lecture SD par DMA vit dans sa propre section ;
    // on l'alloue ici, tot, la ou le tas est peu fragmente (comme les autres
    // patch codes), et non au site d'accroche Dash ou le plus grand bloc libre
    // est reduit. dashReadDmaFn = adresse relogee de dash_readSdDma.
    u32 dashReadDmaFn = 0;
    if ((patchContext.GetGameCode() & 0x00FFFFFF) == GAMECODE_NO_REGION("APD"))
    {
        u32 dmaSize = SECTION_SIZE(dash_readsddma);
        void* dmaAddr = patchContext.GetPatchHeap().Alloc(dmaSize);
        memcpy(dmaAddr, SECTION_START(dash_readsddma), dmaSize);
        dashReadDmaFn = (u32)&dash_readSdDma - (u32)SECTION_START(dash_readsddma) + (u32)dmaAddr;
    }
    u32 patch4Size = SECTION_SIZE(fixcp15);
    void* patch4Address = patchContext.GetPatchHeap().Alloc(patch4Size);
    __patch_cardireadcard_fix_cp15_asm_address = (u32)&fix_cp15_asm - (u32)SECTION_START(fixcp15) + (u32)patch4Address;

    u32 entryAddress = (u32)&patch_cardireadcard_entry - (u32)SECTION_START(patch_cardireadcard) + (u32)patch1Address;

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

    memcpy(patch1Address, SECTION_START(patch_cardireadcard), patch1Size);
    memcpy(patch4Address, SECTION_START(fixcp15), patch4Size);

    // Pokemon Dash (APD*) : le jeu lit son systeme de fichiers via un moteur
    // DMA carte qui lui est propre, hors des fonctions CARDi_* redirigees vers
    // la SD. On intercepte l'assemblage de la commande carte 0xB7 a 0x0206D3A4
    // pour y substituer une lecture SD, dans le contexte d'execution de Dash.
    // v19 : la lecture est par DMA (dashReadDmaFn) et non plus une copie
    // processeur, afin de regenerer l'IRQ carte attendue par le moteur.
    // dashReadDmaFn == 0 => l'alloc precoce a echoue : on n'accroche pas, Dash
    // demarre comme non-patche (temoin d'echec lisible, pas de blx 0).
    if (dashReadDmaFn != 0u
        && (patchContext.GetGameCode() & 0x00FFFFFF) == GAMECODE_NO_REGION("APD"))
    {
        u32 dashSize = SECTION_SIZE(dash_cmd_fix);
        void* dashAddr = patchContext.GetPatchHeap().Alloc(dashSize);
        dbr_fixcp15    = __patch_cardireadcard_fix_cp15_asm_address;
        dbr_remap      = __patch_cardireadcard_rom_offset_to_sd_sector_asm_address;
        dbr_sdread     = dashReadDmaFn;
        dbr_global_ptr = *(u32*)0x0206D470u;   // = 0x020D6144, contexte du moteur
        memcpy(dashAddr, SECTION_START(dash_cmd_fix), dashSize);

        u32 trampolineEntry = (u32)&dash_cmd_patch_entry
                              - (u32)SECTION_START(dash_cmd_fix) + (u32)dashAddr;

        volatile u32* ii = (volatile u32*)0x0206D3A4u;
        // ii[0] : lsr r1, r5, #8  -> inchange (r5 = offset ROM du secteur)
        ii[1] = 0xE1A00005u;   // mov r0, r5   : passe l'offset au trampoline
        u32 blOffset = ((trampolineEntry - ((u32)&ii[2] + 8u)) >> 2) & 0x00FFFFFFu;
        ii[2] = 0xEB000000u | blOffset;
        ii[3] = 0xE28FF058u;   // add pc, pc, #0x58 -> ii+27
        ii[28] = 0xE1A00000u;  // nop : supprime le dernier octet de commande
    }
}
