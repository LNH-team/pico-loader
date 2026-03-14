#include "common.h"
#include "thumbInstructions.h"
#include "CheatEnginePatchCode.h"
#include "CheatEnginePatch.h"

// sdk2-4
static const u32 sVBlankIntrPatternArm0[] = { 0xE92D4000u, 0xE24DD004u, 0xE59F0018u, 0xE5900000u };
static const u32 sVBlankIntrPatternArm1[] = { 0xE92D4008u, 0xE59F0014u, 0xE5900000u, 0xE3500000u };
static const u32 sVBlankIntrPatternThumb0[] = { 0x00000000u, 0xB081B500u, 0x68004804u, 0xD0012800u }; // +4
static const u32 sVBlankIntrPatternThumb1[] = { 0x46C04770u, 0x027FFE1Du, 0x4804B508u, 0x28006800u }; // +8
static const u32 sVBlankIntrPatternThumb2[] = { 0x46C04718u, 0x0000FFFFu, 0x4804B508u, 0x28006800u }; // +8
static const u32 sVBlankIntrPatternThumb3[] = { 0xE51FF004u, 0x037FD67Cu, 0x4804B508u, 0x28006800u }; // +8
static const u32 sVBlankIntrPatternThumb4[] = { 0x46C04770u, 0x02FFFE1Du, 0x4804B508u, 0x28006800u }; // +8
static const u32 sVBlankIntrPatternThumb5[] = { 0xE51FF004u, 0x037FA868u, 0x4804B508u, 0x28006800u }; // +8

// sdk5
static const u32 sVBlankIntrPatternArm2[] = { 0xE92D4008u, 0xE59F2038u, 0xE59F0038u, 0xE5921000u };

bool CheatEnginePatch::FindPatchTarget(PatchContext& patchContext)
{
    _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternArm0, sizeof(sVBlankIntrPatternArm0));
    if (_vblankIrqHandler)
    {
        _foundPattern = sVBlankIntrPatternArm0;
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternArm1, sizeof(sVBlankIntrPatternArm1));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternArm1;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternArm2, sizeof(sVBlankIntrPatternArm2));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternArm2;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb0, sizeof(sVBlankIntrPatternThumb0));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb0;
            _vblankIrqHandler += 1;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb1, sizeof(sVBlankIntrPatternThumb1));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb1;
            _vblankIrqHandler += 2;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb2, sizeof(sVBlankIntrPatternThumb2));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb2;
            _vblankIrqHandler += 2;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb3, sizeof(sVBlankIntrPatternThumb3));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb3;
            _vblankIrqHandler += 2;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb4, sizeof(sVBlankIntrPatternThumb4));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb4;
            _vblankIrqHandler += 2;
        }
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sVBlankIntrPatternThumb5, sizeof(sVBlankIntrPatternThumb5));
        if (_vblankIrqHandler)
        {
            _foundPattern = sVBlankIntrPatternThumb5;
            _vblankIrqHandler += 2;
        }
    }

    if (_vblankIrqHandler)
    {
        LOG_DEBUG("ARM7 VBlankIntr found at 0x%p\n", _vblankIrqHandler);
    }

    return _vblankIrqHandler != nullptr;
}

void CheatEnginePatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_vblankIrqHandler || !_cheats)
        return;

    auto cheatEnginePatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<CheatEnginePatchCode>
    (
        patchContext.GetPatchHeap(),
        _cheats
    );

    if (_foundPattern == sVBlankIntrPatternArm0)
    {
        // push {lr}
        // sub sp, sp, #4
        // ldr r0,=
        // ldr r0, [r0]
        // cmp r0, #0
        // beq 1f
        // bl
        // 1:
        // add sp, sp, #4
        // pop {lr}
        // bx lr
        _vblankIrqHandler[7] = 0xE59F0000; // ldr r0,= address
        _vblankIrqHandler[8] = 0xE12FFF10; // bx r0
        _vblankIrqHandler[9] = (u32)cheatEnginePatchCode->GetCheatEngineFunction(); // address
    }
    else if (_foundPattern == sVBlankIntrPatternArm1)
    {
        // push {r3,lr}
        // ldr r0,=
        // ldr r0, [r0]
        // cmp r0, #0
        // beq 1f
        // bl
        // 1:
        // pop {r3,lr}
        // bx lr
        _vblankIrqHandler[6] = 0xE51FF004; // ldr pc,= address
        _vblankIrqHandler[7] = (u32)cheatEnginePatchCode->GetCheatEngineFunctionArm(); // address
    }
    else if (_foundPattern == sVBlankIntrPatternArm2)
    {
        // push {r3,lr}
        // ldr r2,=
        // ldr r0,=
        // ldr r1, [r2]
        // ldr r3, [r0, #0x60]
        // add r0, r1, #1
        // str r0, [r2]
        // cmp r3, #0
        // beq 1f
        // mov lr, pc
        // bx r3
        // 1:
        // ldr r1,=
        // ldr r0, [r1]
        // orr r0, r0, #1
        // str r0, [r1]
        // pop {r3,lr}
        // bx lr
        _vblankIrqHandler[15] = 0xE51FF004; // ldr pc,= address
        _vblankIrqHandler[16] = (u32)cheatEnginePatchCode->GetCheatEngineFunctionArm(); // address
    }
    else if (_foundPattern == sVBlankIntrPatternThumb0)
    {
        // push {lr}
        // sub sp, sp, #4
        // ldr r0,=
        // ldr r0, [r0]
        // cmp r0, #0
        // beq 1f
        // bl
        // 1:
        // add sp, sp, #4
        // pop {r3}
        // bx r3
        // nop
        ((u16*)_vblankIrqHandler)[8] = THUMB_LDR_PC_IMM(THUMB_R1, 0); // ldr r1,= address
        ((u16*)_vblankIrqHandler)[9] = THUMB_BX(THUMB_R1); // bx r1
        _vblankIrqHandler[10 >> 1] = (u32)cheatEnginePatchCode->GetCheatEngineFunction(); // address
    }
    else if (_foundPattern == sVBlankIntrPatternThumb1
        || _foundPattern == sVBlankIntrPatternThumb2
        || _foundPattern == sVBlankIntrPatternThumb3
        || _foundPattern == sVBlankIntrPatternThumb4
        || _foundPattern == sVBlankIntrPatternThumb5)
    {
        // push {r3,lr}
        // ldr r0,=
        // ldr r0, [r0]
        // cmp r0, #0
        // beq 1f
        // bl
        // 1:
        // pop {r3}
        // pop {r3}
        // bx r3

        // rewrite the function to
        // add r1, pc, #4
        // ldr r0,=
        // ldr r2,= cheatengine_entry_thumb_replace
        // bx r2
        // beq 1f
        // bl
        // 1:
        // mov pc, r4
        // .word cheatengine_entry_thumb_replace

        ((u16*)_vblankIrqHandler)[0] = THUMB_ADD_PC_IMM(THUMB_R1, 4); // add r1, pc, #4
        ((u16*)_vblankIrqHandler)[2] = THUMB_LDR_PC_IMM(THUMB_R2, 8); // ldr r2,= cheatengine_entry_thumb_replace
        ((u16*)_vblankIrqHandler)[3] = THUMB_BX(THUMB_R2); // bx r2
        ((u16*)_vblankIrqHandler)[7] = THUMB_MOV_HIREG(THUMB_HI_PC, THUMB_R4); // mov pc, r4
        _vblankIrqHandler[8 >> 1] = (u32)cheatEnginePatchCode->GetCheatEngineFunctionThumbReplace(); // .word cheatengine_entry_thumb_replace
    }
    else
    {
        LOG_ERROR("ARM7 VBlankIntr signature not implemented\n");
    }
}
