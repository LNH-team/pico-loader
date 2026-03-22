#include "common.h"
#include "CheatEnginePatchCode.h"
#include "CheatEnginePatch.h"

static const u32 sOSiIrqVBlankPattern0[] = { 0xE92D4008u, 0xE59F2038u, 0xE59F0038u, 0xE5921000u };
static const u32 sOSiIrqVBlankPattern1[] = { 0xE92D4000u, 0xE24DD004u, 0xE59F003Cu, 0xE5902060u };

bool CheatEnginePatch::FindPatchTarget(PatchContext& patchContext)
{
    _vblankIrqHandler = patchContext.FindPattern32(sOSiIrqVBlankPattern0, sizeof(sOSiIrqVBlankPattern0));
    if (_vblankIrqHandler)
    {
        _foundPattern = sOSiIrqVBlankPattern0;
    }
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sOSiIrqVBlankPattern1, sizeof(sOSiIrqVBlankPattern1));
        if (_vblankIrqHandler)
        {
            _foundPattern = sOSiIrqVBlankPattern1;
        }
    }

    if (_vblankIrqHandler)
    {
        LOG_DEBUG("ARM7 OSi_IrqVBlank found at 0x%p\n", _vblankIrqHandler);
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

    int patchOffset;
    if (_foundPattern == sOSiIrqVBlankPattern0)
    {
        patchOffset = 15;
    }
    else if (_foundPattern == sOSiIrqVBlankPattern1)
    {
        patchOffset = 16;
    }
    else
    {
        LOG_ERROR("ARM7 OSi_IrqVBlank signature not implemented\n");
        return;
    }

    _vblankIrqHandler[patchOffset + 0] = 0xE51FF004; // ldr pc,= address
    _vblankIrqHandler[patchOffset + 1] = (u32)cheatEnginePatchCode->GetCheatEngineFunctionArm(); // address
    LOG_DEBUG("Cheats enabled\n");
}
