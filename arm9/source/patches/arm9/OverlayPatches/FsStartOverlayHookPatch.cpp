#include "common.h"
#include "ArmHelper.h"
#include "../../PatchContext.h"
#include "thumbInstructions.h"
#include "FsStartOverlayHookPatchAsm.h"
#include "FsStartOverlayHookPatch.h"

static const u32 sFSStartOverlayPatternSdk3[] = { 0xE59F10DCu, 0xE1A04000u, 0xE1D100B0u, 0xE3500002u }; // -0xC
static const u32 sFSStartOverlayPatternSdk4[] = { 0xE59F10D0u, 0xE1A04000u, 0xE1D100B0u, 0xE3500002u }; // -0xC
static const u32 sFSStartOverlayPatternSdk4Thumb[] = { 0x481F1C06u, 0x28028800u, 0x69E8D121u, 0x0E012700u }; // -8
static const u32 sFSStartOverlayPattern[] = { 0xE3500001u, 0x0A00001Cu, 0xE595001Cu, 0xE3A03000u }; // -0x14
static const u32 sFSStartOverlayPatternThumb[] = { 0x69E8D023u, 0x0E012600u, 0x42082002u, 0x491BD013u }; // -0x10
static const u32 sFSStartOverlayPatternThumbHybrid[] = { 0x68691C07u, 0x42814828u, 0x4828D30Cu, 0xD2094281u }; // -8

void FsStartOverlayHookPatch::TryPattern(PatchContext& patchContext, const u32* pattern, u32 startOffset)
{
    _fsStartOverlay = patchContext.FindPattern32(pattern, 16);
    if (_fsStartOverlay)
    {
        _fsStartOverlay = (u32*)((u8*)_fsStartOverlay + startOffset);
        _foundPattern = pattern;
    }
}

bool FsStartOverlayHookPatch::FindPatchTarget(PatchContext& patchContext)
{
    if (!_patchHead)
    {
        // no patches
        LOG_DEBUG("No overlay patches, not searching for FS_StartOverlay\n");
        return true;
    }

    if (patchContext.GetSdkVersion().GetMajor() == 5)
    {
        TryPattern(patchContext, sFSStartOverlayPattern, -0x14);
        if (!_fsStartOverlay)
        {
            TryPattern(patchContext, sFSStartOverlayPatternThumb, -0x10);
            if (!_fsStartOverlay)
            {
                TryPattern(patchContext, sFSStartOverlayPatternThumbHybrid, -0x8);
            }

            if (_fsStartOverlay)
            {
                _thumb = true;
            }
        }
    }
    else
    {
        // sdk 2-4
        if (patchContext.GetSdkVersion().GetMajor() <= 3)
        {
            TryPattern(patchContext, sFSStartOverlayPatternSdk3, -0xC);
        }

        if (!_fsStartOverlay)
        {
            TryPattern(patchContext, sFSStartOverlayPatternSdk4, -0xC);
        }

        if (!_fsStartOverlay)
        {
            TryPattern(patchContext, sFSStartOverlayPatternSdk4Thumb, -0x8);
            if (_fsStartOverlay)
            {
                _thumb = true;
            }
        }
    }

    if (_fsStartOverlay)
    {
        LOG_DEBUG("FS_StartOverlay found at 0x%p\n", _fsStartOverlay);
    }
    else
    {
        LOG_WARNING("FS_StartOverlay not found\n");
    }

    return _fsStartOverlay != nullptr;
}

void FsStartOverlayHookPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_fsStartOverlay)
        return;

    if (!_patchHead) // no patches
        return;

    const void* firstPatch = _patchHead->InsertPatch(patchContext);

    if (!firstPatch)
        return;

    fsstartoverlayhook_hookFuncAddress = (u32)firstPatch;

    u32 patchOffset;
    u32 dcFlushRangeCallOffset;
    if (_thumb)
    {
        if (_foundPattern == sFSStartOverlayPatternThumbHybrid)
        {
            patchOffset = 0x90;
            dcFlushRangeCallOffset = 0x92;
        }
        else if (_foundPattern == sFSStartOverlayPatternSdk4Thumb)
        {
            patchOffset = 0x68;
            dcFlushRangeCallOffset = 0x6C;
        }
        else if (_foundPattern == sFSStartOverlayPatternThumb)
        {
            patchOffset = 0x6C;
            dcFlushRangeCallOffset = 0x70;
        }
        else
        {
            LOG_ERROR("Unknown Thumb FS_StartOverlay\n");
            return;
        }
    }
    else
    {
        if (_foundPattern == sFSStartOverlayPatternSdk3)
        {
            patchOffset = 0xAC;
            dcFlushRangeCallOffset = 0xB4;
        }
        else if (_foundPattern == sFSStartOverlayPatternSdk4 || _foundPattern == sFSStartOverlayPattern)
        {
            patchOffset = 0xAC;
            dcFlushRangeCallOffset = 0xB0;
        }
        else
        {
            LOG_ERROR("Unknown Arm FS_StartOverlay\n");
            return;
        }
    }

    void* blDcFlushRange = (void*)((u8*)_fsStartOverlay + dcFlushRangeCallOffset);
    if (_thumb)
    {
        fsstartoverlayhook_dcFlushRangeAddress = ArmHelper::GetThumbCallAddress(blDcFlushRange);
    }
    else
    {
        fsstartoverlayhook_dcFlushRangeAddress = ArmHelper::GetArmCallAddress(blDcFlushRange);
    }

    u32 patch1Size = SECTION_SIZE(fsstartoverlayhook);
    void* patch1Address = patchContext.GetPatchHeap().Alloc(patch1Size);
    u32 entryAddress = (u32)&fsstartoverlayhook_entry - (u32)SECTION_START(fsstartoverlayhook) + (u32)patch1Address;
    memcpy(patch1Address, SECTION_START(fsstartoverlayhook), patch1Size);

    if (_thumb)
    {
        *(u16*)((u8*)_fsStartOverlay + patchOffset + 0) = THUMB_LDR_PC_IMM(THUMB_R0, 0);
        *(u16*)((u8*)_fsStartOverlay + patchOffset + 2) = THUMB_BLX(THUMB_R0);
        *(u32*)((u8*)_fsStartOverlay + patchOffset + 4) = entryAddress;
    }
    else
    {
        *(u32*)((u8*)_fsStartOverlay + patchOffset + 0) = 0xE59F0000; // ldr r0,= entryAddress
        *(u32*)((u8*)_fsStartOverlay + patchOffset + 4) = 0xE12FFF30; // blx r0
        *(u32*)((u8*)_fsStartOverlay + patchOffset + 8) = entryAddress;
    }
}
