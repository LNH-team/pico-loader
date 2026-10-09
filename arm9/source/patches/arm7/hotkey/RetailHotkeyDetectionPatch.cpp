#include "common.h"
#include "thumbInstructions.h"
#include "RetailHotkeyDetectionPatchCode.h"
#include "RetailHotkeyDetectionPatch.h"

static const u32 sOSiIrqVBlankPattern0[] = { 0xE92D4008u, 0xE59F2038u, 0xE59F0038u, 0xE5921000u };
static const u32 sOSiIrqVBlankPattern1[] = { 0xE92D4000u, 0xE24DD004u, 0xE59F003Cu, 0xE5902060u };
static const u32 sOSiIrqVBlankPatternThumb0[] = { 0x4809B508u, 0x69024909u, 0x1C406808u, 0x2A006008u };

bool RetailHotkeyDetectionPatch::FindPatchTarget(PatchContext& patchContext)
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
    if (!_vblankIrqHandler)
    {
        _vblankIrqHandler = patchContext.FindPattern32(sOSiIrqVBlankPatternThumb0, sizeof(sOSiIrqVBlankPatternThumb0));
        if (_vblankIrqHandler)
        {
            _foundPattern = sOSiIrqVBlankPatternThumb0;
            _thumb = true;
        }
    }

    if (_vblankIrqHandler)
    {
        LOG_DEBUG("ARM7 retail hotkey VBlank target found at 0x%p\n", _vblankIrqHandler);
    }
    else
    {
        LOG_WARNING("ARM7 VBlank target not found; retail hotkey detector disabled\n");
    }

    // Optional patch: never prevent a retail game from booting just because
    // this detector is unavailable for its SDK/build.
    return true;
}

void RetailHotkeyDetectionPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_vblankIrqHandler)
        return;

    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<RetailHotkeyDetectionPatchCode>
    (
        patchContext.GetPatchHeap()
    );

    int patchOffset;
    if (_foundPattern == sOSiIrqVBlankPattern0)
    {
        patchOffset = 0x3C;
    }
    else if (_foundPattern == sOSiIrqVBlankPattern1)
    {
        patchOffset = 0x40;
    }
    else if (_foundPattern == sOSiIrqVBlankPatternThumb0)
    {
        patchOffset = 0x20;
    }
    else
    {
        return;
    }

    if (_thumb)
    {
        *(u16*)((u8*)_vblankIrqHandler + patchOffset + 0) = THUMB_LDR_PC_IMM(THUMB_R1, 0);
        *(u16*)((u8*)_vblankIrqHandler + patchOffset + 2) = THUMB_BX(THUMB_R1);
        *(u32*)((u8*)_vblankIrqHandler + patchOffset + 4) = (u32)patchCode->GetEntryFunction();
    }
    else
    {
        *(u32*)((u8*)_vblankIrqHandler + patchOffset + 0) = 0xE51FF004;
        *(u32*)((u8*)_vblankIrqHandler + patchOffset + 4) = (u32)patchCode->GetEntryFunctionArm();
    }

    LOG_DEBUG("ARM7 retail hotkey detection hook enabled\n");
}
