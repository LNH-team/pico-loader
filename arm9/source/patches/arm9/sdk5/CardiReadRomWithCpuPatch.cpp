#include "common.h"
#include "patches/PatchContext.h"
#include "fileInfo.h"
#include "thumbInstructions.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm9/RomOffsetToSdSectorPatchCode.h"
#include "patches/OffsetToSectorRemapPatchCode.h"
#include "CardiReadRomWithCpuPatchAsm.h"
#include "patches/arm9/FixCp15Asm.h"
#include "CardiReadRomWithCpuPatch.h"

static const u32 sCARDiReadRomWithCPUPattern500753B[] = { 0xE92D4FF8u, 0xE24DD008u, 0xE59F00FCu, 0xE1A08003u };
static const u32 sCARDiReadRomWithCPUPatternThumb[] = { 0xB085B5F0u, 0x1C1D482Du, 0x91006840u, 0x95049201u };
static const u32 sCARDiReadRomWithCPUPatternThumb5017537[] = { 0xB085B5F0u, 0x1C1D482Du, 0x91006800u, 0x95049201u };
// static const u32 sCARDiReadRomWithCPUPatternThumb5004E85[] = { 0xB085B5F0u, 0x1C1D482Bu, 0x910068C0u, 0x95049201u };
static const u32 sCARDiReadRomWithCPUPatternThumb5007538[] = { 0xB085B5F0u, 0x1C1D482Bu, 0x91006800u, 0x95049201u };
static const u32 sCARDiReadRomWithCPUPatternThumb5037531[] = { 0xB086B5F8u, 0x1C1D482Fu, 0x91006800u, 0x482E9003u };
static const u32 sCARDiReadRomWithCPUPatternThumbHybrid5037531[] = { 0xB086B5F8u, 0x1C1D482Fu, 0x91006840u, 0x482E9003u };

void CardiReadRomWithCpuPatch::TryPattern(PatchContext& patchContext, const u32* pattern)
{
    _cardiReadRomWithCpu = patchContext.FindPattern32(pattern, 16);
    if (_cardiReadRomWithCpu)
    {
        _foundPattern = pattern;
    }
}

bool CardiReadRomWithCpuPatch::FindPatchTarget(PatchContext& patchContext)
{
    TryPattern(patchContext, sCARDiReadRomWithCPUPattern500753B);
    if (!_cardiReadRomWithCpu)
    {
        TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumb5037531);
        if (!_cardiReadRomWithCpu)
        {
            TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumbHybrid5037531);
        }
        if (!_cardiReadRomWithCpu)
        {
            TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumb5017537);
        }
        if (!_cardiReadRomWithCpu)
        {
            TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumb5007538);
        }
        if (!_cardiReadRomWithCpu)
        {
            TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumb);
        }
        // if (!_cardiReadRomWithCpu)
        // {
        //     TryPattern(patchContext, sCARDiReadRomWithCPUPatternThumb5004E85);
        // }
        if (_cardiReadRomWithCpu)
        {
            _thumb = true;
        }
    }

    if (_cardiReadRomWithCpu)
    {
        LOG_DEBUG("Found CARDi_ReadRomWithCPU at %p\n", _cardiReadRomWithCpu);
        if (_thumb)
        {
            LOG_DEBUG("thumb CARDi_ReadRomWithCPU\n");
        }
    }
    else
    {
        LOG_WARNING("CARDi_ReadRomWithCPU not found\n");
    }
    return _cardiReadRomWithCpu != nullptr;
}

void CardiReadRomWithCpuPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiReadRomWithCpu)
        return;

    auto loaderPlatform = patchContext.GetLoaderPlatform();
    if (loaderPlatform->HasRomReads())
    {
        auto romReadPatchCode = loaderPlatform->CreateRomReadPatchCode(
            patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
        auto offsetToSectorRemapPatchCode = patchContext.GetPatchCodeCollection().GetOrAddSharedPatchCode([&]
        {
            return new OffsetToSectorRemapPatchCode(patchContext.GetPatchHeap());
        });
        __patch_cardireadromwithcpu_rom_offset_to_sd_sector_asm_address = (u32)offsetToSectorRemapPatchCode->GetRemapFunction();
        __patch_cardireadromwithcpu_sdread_asm_address = (u32)romReadPatchCode->GetReadSectorsFunction();
    }
    else
    {
        auto sdReadPatchCode = loaderPlatform->CreateSdReadPatchCode(
            patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
        auto romOffsetToSdSectorPatchCode = patchContext.GetPatchCodeCollection().GetOrAddSharedPatchCode([&]
        {
            return new RomOffsetToSdSectorPatchCode(patchContext.GetPatchHeap(), SHARED_ROM_FILE_INFO);
        });
        __patch_cardireadromwithcpu_rom_offset_to_sd_sector_asm_address = (u32)romOffsetToSdSectorPatchCode->GetRemapFunction();
        __patch_cardireadromwithcpu_sdread_asm_address = (u32)sdReadPatchCode->GetReadSectorsFunction();
    }

    u32 patch1Size = SECTION_SIZE(patch_cardireadromwithcpu);
    void* patch1Address = patchContext.GetPatchHeap().Alloc(patch1Size);
    u32 patch4Size = SECTION_SIZE(fixcp15);
    void* patch4Address = patchContext.GetPatchHeap().Alloc(patch4Size);
    __patch_cardireadromwithcpu_fix_cp15_asm_address = (u32)&fix_cp15_asm - (u32)SECTION_START(fixcp15) + (u32)patch4Address;
    u32 entryAddress = (u32)&patch_cardireadromwithcpu_entry - (u32)SECTION_START(patch_cardireadromwithcpu) + (u32)patch1Address;
    if (_thumb)
    {
        bool old = _foundPattern == sCARDiReadRomWithCPUPatternThumb5017537
            || _foundPattern == sCARDiReadRomWithCPUPatternThumb5007538
            // || _foundPattern == sCARDiReadRomWithCPUPatternThumb5004E85
            || _foundPattern == sCARDiReadRomWithCPUPatternThumb;
        u32 patchOffset = old ? 0x48 : 0x4C;
        u32 returnOffset = old ? 0x26 : 0x28;
        patch_cardireadromwithcpu_return_offset = THUMB_MOVS_IMM(THUMB_R0, returnOffset);
        patch_cardireadromwithcpu_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R1); // src
        patch_cardireadromwithcpu_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R4); // dst
        patch_cardireadromwithcpu_mov_actual_dst_to_r3 = THUMB_LDR_SP_IMM(THUMB_R3, 4);
        patch_cardireadromwithcpu_mov_left_to_read_to_r2 = THUMB_MOVS_REG(THUMB_R2, THUMB_R5);

        *(u16*)((u8*)_cardiReadRomWithCpu + patchOffset - 2) = THUMB_LDR_SP_IMM(THUMB_R1, 8); // load rom source to r1 instead of r0
        *(u16*)((u8*)_cardiReadRomWithCpu + patchOffset + 0) = 0x4800; // ldr r0,= entryAddress
        *(u16*)((u8*)_cardiReadRomWithCpu + patchOffset + 2) = THUMB_BLX(THUMB_R0);
        *(u32*)((u8*)_cardiReadRomWithCpu + patchOffset + 4) = entryAddress;
    }
    else
    {
        u32 patchOffset = 0x78;
        patch_cardireadromwithcpu_return_offset = THUMB_MOVS_IMM(THUMB_R0, 0x28);
        patch_cardireadromwithcpu_mov_src_to_r0 = THUMB_MOVS_REG(THUMB_R0, THUMB_R6); // src
        patch_cardireadromwithcpu_mov_dst_to_r1 = THUMB_MOVS_REG(THUMB_R1, THUMB_R5); // dst
        patch_cardireadromwithcpu_mov_actual_dst_to_r3 = THUMB_MOV_HIREG(THUMB_R3, THUMB_HI_R10);
        patch_cardireadromwithcpu_mov_left_to_read_to_r2 = THUMB_MOV_HIREG(THUMB_R2, THUMB_HI_R8);

        *(u32*)((u8*)_cardiReadRomWithCpu + patchOffset + 0) = 0xE59F0000; // ldr r0,= entryAddress
        *(u32*)((u8*)_cardiReadRomWithCpu + patchOffset + 4) = 0xE12FFF30; // blx r0
        *(u32*)((u8*)_cardiReadRomWithCpu + patchOffset + 8) = entryAddress;
    }
    memcpy(patch1Address, SECTION_START(patch_cardireadromwithcpu), patch1Size);
    memcpy(patch4Address, SECTION_START(fixcp15), patch4Size);
}
