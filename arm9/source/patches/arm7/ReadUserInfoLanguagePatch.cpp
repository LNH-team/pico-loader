#include "common.h"
#include "ArmHelper.h"
#include "sharedMemory.h"
#include "ndsHeader.h"
#include "../PatchContext.h"
#include "ReadUserInfoLanguagePatchCode.h"
#include "ReadUserInfoLanguagePatch.h"

// ReadUserInfo copies the valid user settings to nvramUserInfo with:
//     ldr r1, =nvramUserInfo  ; somewhere in the instructions before
//     add r0, rX, r0, lsl #8  ; one of the two 256 byte copies read from flash
//     mov r2, #0x74           ; sizeof(NVRAMConfig)
//     bl MI_CpuCopy32
//     b next                  ; skips the fill and clear paths
#define ARM_MOV_R2_USER_INFO_SIZE           0xE3A02074u
#define ARM_ADD_R0_RX_R0_LSL_8_MASK         0xFFF0FFFFu
#define ARM_ADD_R0_RX_R0_LSL_8              0xE0800400u
#define ARM_LDR_R1_PC_RELATIVE_MASK         0xFFFFF000u
#define ARM_LDR_R1_PC_RELATIVE              0xE59F1000u
#define ARM_B                               0xEAu
#define ARM_MOV_R2_PC                       0xE1A0200Fu
#define ARM_LDR_PC_NEXT_WORD                0xE51FF004u

#define NTR_NVRAM_USER_INFO                 0x027FFC80u
#define TWL_NVRAM_USER_INFO                 0x02FFFC80u

static bool isUserInfoLoad(const u32* instruction, const u32* arm7Start, const u32* arm7End)
{
    if ((*instruction & ARM_LDR_R1_PC_RELATIVE_MASK) != ARM_LDR_R1_PC_RELATIVE)
    {
        return false;
    }

    auto literal = (const u32*)((u32)instruction + 8 + (*instruction & 0xFFF));
    if (literal < arm7Start || literal >= arm7End)
    {
        return false;
    }

    return *literal == NTR_NVRAM_USER_INFO || *literal == TWL_NVRAM_USER_INFO;
}

bool ReadUserInfoLanguagePatch::FindPatchTarget(PatchContext& patchContext)
{
    auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
    auto arm7Start = (u32*)romHeader->arm7LoadAddress;
    auto arm7End = (u32*)(romHeader->arm7LoadAddress + romHeader->arm7Size);
    _copySizeInstruction = nullptr;
    for (u32* ptr = arm7Start + 6; ptr + 2 < arm7End; ptr++)
    {
        if (ptr[0] != ARM_MOV_R2_USER_INFO_SIZE ||
            !ArmHelper::IsArmUnconditionalBl(ptr[1]) ||
            (ptr[2] >> 24) != ARM_B)
        {
            continue;
        }

        // Relative to the replaced "b next", which is where r2 points to at runtime.
        // It has to fit the 8 bit immediate in the patch code.
        u32 returnOffset = ArmHelper::GetArmCallOffset(ptr[2]);
        if (returnOffset > 0xFF)
        {
            continue;
        }

        // Older sdks load nvramUserInfo between the add and the mov
        int addIndex = ((ptr[-1] & ARM_ADD_R0_RX_R0_LSL_8_MASK) == ARM_ADD_R0_RX_R0_LSL_8) ? -1 : -2;
        if ((ptr[addIndex] & ARM_ADD_R0_RX_R0_LSL_8_MASK) != ARM_ADD_R0_RX_R0_LSL_8)
        {
            continue;
        }

        for (int i = -1; i >= -6; i--)
        {
            if (isUserInfoLoad(&ptr[i], arm7Start, arm7End))
            {
                _copySizeInstruction = ptr;
                _returnOffset = returnOffset;
                break;
            }
        }

        if (_copySizeInstruction)
        {
            break;
        }
    }

    if (_copySizeInstruction)
    {
        LOG_DEBUG("ARM7 ReadUserInfo copy found at 0x%p\n", _copySizeInstruction);
    }
    else
    {
        // Not being able to apply the language must not prevent booting the rom
        LOG_WARNING("ARM7 ReadUserInfo copy not found, game language not applied\n");
    }

    return true;
}

void ReadUserInfoLanguagePatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_copySizeInstruction)
    {
        return;
    }

    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<ReadUserInfoLanguagePatchCode>
    (
        patchContext.GetPatchHeap(),
        _language,
        _returnOffset
    );

    // No relative branch, because this code runs from an autoload address
    _copySizeInstruction[0] = ARM_MOV_R2_PC;
    _copySizeInstruction[1] = ARM_LDR_PC_NEXT_WORD;
    _copySizeInstruction[2] = (u32)patchCode->GetCopyUserInfoFunction();

    LOG_DEBUG("Game language %d applied to ARM7 ReadUserInfo\n", _language);
}
