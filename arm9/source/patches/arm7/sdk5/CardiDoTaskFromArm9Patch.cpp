#include "common.h"
#include "patches/PatchContext.h"
#include "thumbInstructions.h"
#include "fileInfo.h"
#include "patches/arm7/ReadSavePatchCode.h"
#include "patches/arm7/WriteSavePatchCode.h"
#include "patches/arm7/VerifySavePatchCode.h"
#include "patches/SaveOffsetToSdSectorPatchCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm7/CardiTaskThreadPatchAsm.h"
#include "CardiDoTaskFromArm9Patch.h"

static const u32 sCARDiDoTaskFromARM9Pattern5007538[] = { 0xE92D4038u, 0xE59F4190u, 0xE3A01001u, 0xE5943000u }; // this one has branches instead of an offset table
static const u32 sCARDiDoTaskFromARM9Pattern5017537[] = { 0xE92D4070u, 0xE59F5168u, 0xE3A01001u, 0xE5953000u };
static const u32 sCARDiDoTaskFromARM9Pattern5057530[] = { 0xE92D4070u, 0xE59F5174u, 0xE3A01001u, 0xE5953000u };

static const u32 sCARDiDoTaskFromARM9Pattern5017537Thumb[] = { 0x4C3FB538u, 0x68214A3Fu, 0x23016E92u, 0x40936D88u };
static const u32 sCARDiDoTaskFromARM9Pattern5037531Thumb[] = { 0x4C3EB538u, 0x68214A3Eu, 0x23016E92u, 0x40936D88u };
static const u32 sCARDiDoTaskFromARM9Pattern5057530Thumb[] = { 0x4C40B538u, 0x68214A40u, 0x23016E92u, 0x40936D88u };

void CardiDoTaskFromArm9Patch::TryPattern(PatchContext& patchContext, const u32* pattern)
{
    _cardiDoTaskFromArm9 = patchContext.FindPattern32(pattern, 16);
    if (_cardiDoTaskFromArm9)
    {
        _foundPattern = pattern;
    }
}

bool CardiDoTaskFromArm9Patch::FindPatchTarget(PatchContext& patchContext)
{
    if (patchContext.GetSdkVersion() <= 0x5027535)
        TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5007538);
    if (!_cardiDoTaskFromArm9 && patchContext.GetSdkVersion() >= 0x5017537 && patchContext.GetSdkVersion() <= 0x5057534)
        TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5017537);
    if (!_cardiDoTaskFromArm9 && patchContext.GetSdkVersion() >= 0x5057530)
        TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5057530);
    if (!_cardiDoTaskFromArm9)
    {
        TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5057530Thumb);
        if (!_cardiDoTaskFromArm9)
            TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5037531Thumb);
        if (!_cardiDoTaskFromArm9)
            TryPattern(patchContext, sCARDiDoTaskFromARM9Pattern5017537Thumb);
        if (_cardiDoTaskFromArm9)
        {
            _thumb = true;
        }
    }
    if (_cardiDoTaskFromArm9)
    {
        LOG_DEBUG("CARDi_DoTaskFromARM9 found at 0x%p\n", _cardiDoTaskFromArm9);
    }
    return _cardiDoTaskFromArm9 != nullptr;
}

void CardiDoTaskFromArm9Patch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiDoTaskFromArm9)
        return;

    u32 patch1Size = SECTION_SIZE(patch_carditaskthread);
    void* patch1Address = patchContext.GetPatchHeap().Alloc(patch1Size);
    auto loaderPlatform = patchContext.GetLoaderPlatform();
    auto readPatchCode = loaderPlatform->CreateSdReadPatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
    auto writePatchCode = loaderPlatform->CreateSdWritePatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
    auto sectorRemapPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<SaveOffsetToSdSectorPatchCode>(
        patchContext.GetPatchHeap(), SHARED_SAVE_FILE_INFO);
    void* tmpBuffer = patchContext.GetPatchHeap().Alloc(512);
    auto readSavePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<ReadSavePatchCode>(
        patchContext.GetPatchHeap(),
        sectorRemapPatchCode,
        readPatchCode,
        tmpBuffer
    );
    auto writeSavePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<WriteSavePatchCode>(
        patchContext.GetPatchHeap(),
        sectorRemapPatchCode,
        readPatchCode,
        writePatchCode,
        tmpBuffer
    );
    auto verifySavePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<VerifySavePatchCode>(
        patchContext.GetPatchHeap(),
        sectorRemapPatchCode,
        readPatchCode,
        tmpBuffer
    );

    __patch_carditaskthread_readsave_asm_address = (u32)readSavePatchCode->GetReadSaveFunction();
    __patch_carditaskthread_writesave_asm_address = (u32)writeSavePatchCode->GetWriteSaveFunction();
    __patch_carditaskthread_verifysave_asm_address = (u32)verifySavePatchCode->GetVerifySaveFunction();
    if (loaderPlatform->GetPlatformType() == LoaderPlatformType::Slot1)
    {
        // Test REG_EXMEMSTAT bit 11
        __patch_carditaskthread_lsls_exmemstat_bit_to_r1 = THUMB_LSLS_IMM(THUMB_R1, THUMB_R1, 31 - 11);
    }
    else
    {
        // Test REG_EXMEMSTAT bit 7
        __patch_carditaskthread_lsls_exmemstat_bit_to_r1 = THUMB_LSLS_IMM(THUMB_R1, THUMB_R1, 31 - 7);
    }

    u32 patchOffset;

    if (_thumb)
    {
        patchOffset = _foundPattern == sCARDiDoTaskFromARM9Pattern5057530Thumb ? 0x3A : 0x36;
        u32 entryAddress = (u32)&__patch_carditaskthread_entry - (u32)SECTION_START(patch_carditaskthread) + (u32)patch1Address;
        __patch_carditaskthread_mov_cardicommon_to_r4 = THUMB_NOP;
        __patch_carditaskthread_mov_command_to_r0 = THUMB_NOP;
        __patch_carditaskthread_successoffset = *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x0C) + 9;
        __patch_carditaskthread_failoffset = *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x14) + 9;

        *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x00) = THUMB_LDR_PC_IMM(THUMB_R1, 4);
        *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x02) = THUMB_MOV_HIREG(THUMB_HI_LR, THUMB_HI_PC);
        *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x04) = THUMB_BX(THUMB_R1);
        *(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x06) = entryAddress;
    }
    else
    {
        if (_foundPattern == sCARDiDoTaskFromARM9Pattern5007538)
        {
            // 0x4C = cmp r0, #0xF
            patchOffset = 0x4C;

            __patch_carditaskthread_successoffset = 4;
            __patch_carditaskthread_failoffset = ((*(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x08) & 0xFFFFFF) << 2) + 4;
            __patch_carditaskthread_mov_cardicommon_to_r4 = THUMB_NOP; // already in r4
            __patch_carditaskthread_mov_command_to_r0 = THUMB_NOP;
        }
        else
        {
            // 0x58 = add r0, pc, r0, lsl #1
            if (_foundPattern == sCARDiDoTaskFromARM9Pattern5057530)
                patchOffset = 0x58;
            else
                patchOffset = 0x54;

            __patch_carditaskthread_successoffset = *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x0C) + 4;
            __patch_carditaskthread_failoffset = *(u16*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x14) + 4;
            __patch_carditaskthread_mov_cardicommon_to_r4 = THUMB_MOVS_REG(THUMB_R4, THUMB_R5);
            __patch_carditaskthread_mov_command_to_r0 = THUMB_NOP;
        }

        u32 entryAddress = (u32)&__patch_carditaskthread_entry - (u32)SECTION_START(patch_carditaskthread) + (u32)patch1Address;

        *(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x00) = 0xe59f1004; // ldr r1,= entryAddress
        *(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x04) = 0xe1a0e00f; // mov lr, pc
        *(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x08) = 0xe12fff11; // bx r1
        *(u32*)((u8*)_cardiDoTaskFromArm9 + patchOffset + 0x0C) = entryAddress;
    }
    memcpy(patch1Address, SECTION_START(patch_carditaskthread), patch1Size);
}