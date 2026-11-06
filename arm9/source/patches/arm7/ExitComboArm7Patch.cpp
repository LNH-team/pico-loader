#include "common.h"
#include "patches/PatchContext.h"
#include "sharedMemory.h"
#include "ndsHeader.h"
#include "ExitComboArm7PatchCode.h"
#include "ReadSavePatchCode.h"
#include "WriteSavePatchCode.h"
#include "VerifySavePatchCode.h"
#include "ExitComboArm7Patch.h"

// The combo comes from the launcher through the loader header, so it is checked
// here rather than trusted. The same rules the launcher applies: two to four of
// the twelve real buttons, and SELECT among them, so a game's own chord cannot
// become the exit. Anything else keeps the built-in combo.
static u32 ValidateExitCombo(u16 exitCombo)
{
    constexpr u16 kValidKeys = 0x0FFF; // A B Select Start Right Left Up Down R L X Y
    constexpr u16 kSelectKey = 0x0004;
    constexpr u16 kDefaultCombo = 0x0314; // L | R | Right | Select

    exitCombo &= kValidKeys;
    u32 keyCount = 0;
    for (u16 bit = 1; bit != 0x1000; bit <<= 1)
    {
        if (exitCombo & bit)
            keyCount++;
    }
    if ((exitCombo & kSelectKey) == 0 || keyCount < 2 || keyCount > 4)
    {
        return kDefaultCombo;
    }
    return exitCombo;
}

// Tail of the SDK irq dispatcher, the same four word sequence used on the arm9
// side: ldr r1, [pc, #8]; ldr r0, [r1, r0, lsl #2]; ldr lr, [pc, #4]; bx r0.
// The irq table address and the irq return address literals follow it.
static const u32 sIrqDispatchTailPattern[] = { 0xE59F1008u, 0xE7910100u, 0xE59FE004u, 0xE12FFF10u };

// Where the SDK's nested irq handler calls a handler out of the same table:
// ldr r1, [ip, #0x210]; stmfd sp!, {r1}; add lr, pc, #0; bx r0. The microphone
// sampling puts that handler in the irq vector, and a game that keeps sampling,
// Zelda Phantom Hourglass for one, stops passing the dispatcher tail.
static const u32 sPrioDispatchPattern[] = { 0xE59C1210u, 0xE92D0002u, 0xE28FE000u, 0xE12FFF10u };

// Entry of that handler, a little before its dispatch point:
// mov ip, #0x04000000; add r1, ip, #0x208; ldrh r0, [r1]; tst r0, r0.
static const u32 sPrioHandlerEntryPattern[] = { 0xE3A0C301u, 0xE28C1F82u, 0xE1D100B0u, 0xE1100000u };

bool ExitComboArm7Patch::FindPatchTarget(PatchContext& patchContext)
{
    if (_searched)
        return true;

    _searched = 1;
    _dispatchTail = patchContext.FindPattern32(sIrqDispatchTailPattern, sizeof(sIrqDispatchTailPattern));
    if (_dispatchTail)
    {
        LOG_DEBUG("Arm7 irq dispatch tail found at 0x%p\n", _dispatchTail);
    }
    else
    {
        LOG_DEBUG("Arm7 irq dispatch tail not found\n");
    }

    // The stub gets back into the nested handler through the irq vector, which
    // holds the handler while it is in use, plus the distance from its entry to
    // the instruction after the call. So the hook needs the entry as well.
    _prioDispatch = nullptr;
    u32* prioDispatch = patchContext.FindPattern32(sPrioDispatchPattern, sizeof(sPrioDispatchPattern));
    if (prioDispatch)
    {
        auto romHeader = (const nds_header_ntr_t*)TWL_SHARED_MEMORY->ntrSharedMem.romHeader;
        u32* lowest = (u32*)romHeader->arm7LoadAddress;
        if (prioDispatch - lowest > 0x100)
            lowest = prioDispatch - 0x100;
        for (u32* entry = prioDispatch - 4; entry >= lowest; entry--)
        {
            if (entry[0] == sPrioHandlerEntryPattern[0] && entry[1] == sPrioHandlerEntryPattern[1] &&
                entry[2] == sPrioHandlerEntryPattern[2] && entry[3] == sPrioHandlerEntryPattern[3])
            {
                _prioDispatch = prioDispatch;
                _prioReturnOffset = (u32)(prioDispatch + 4) - (u32)entry;
                break;
            }
        }
    }
    if (_prioDispatch)
    {
        LOG_DEBUG("Arm7 nested irq dispatch found at 0x%p, return offset 0x%X\n", _prioDispatch, _prioReturnOffset);
    }
    else if (prioDispatch)
    {
        // The game has the nested handler but it cannot be hooked. While it is
        // in use the combo would get no answer from the arm7 and the arm9 would
        // wait for good, so the exit is left out instead.
        LOG_DEBUG("Arm7 nested irq dispatch entry not found, no exit combo\n");
        _dispatchTail = nullptr;
    }
    else
    {
        LOG_DEBUG("Arm7 nested irq dispatch not found\n");
    }
    return true; // optional patch
}

void ExitComboArm7Patch::ApplyPatch(PatchContext& patchContext)
{
    if (!_dispatchTail)
        return;

    // unlike the mandatory patches this one gives up instead of halting
    // when the heap is exhausted
    if (!patchContext.GetPatchHeap().CanAlloc(SECTION_SIZE(patch_exitcombo_arm7)))
    {
        LOG_DEBUG("No arm7 patch heap space for the exit combo\n");
        return;
    }

    u32 blockAddress = (u32)&TWL_SHARED_MEMORY->exitRequest;
    if (!patchContext.GetSdkVersion().IsTwlSdk())
    {
        // SDK2-4 games access shared memory through the ntr mirror
        blockAddress -= 0x800000;
    }

    // The save code reaches the sd card through these, set by the save patch applied
    // before this one, or 0 when the game got none. All three save routines use the
    // same read.
    u32 sdRead = readsave_sdread_asm_address;
    u32 sdWrite = writesave_sdwrite_asm_address;

    auto exitComboPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<ExitComboArm7PatchCode>
    (
        patchContext.GetPatchHeap(),
        blockAddress,
        sdRead,
        sdWrite,
        _dispatchTail[4], // irq table address literal
        _dispatchTail[5], // irq return address literal
        _prioReturnOffset,
        ValidateExitCombo(_exitCombo) // 0, an older launcher's header, keeps the default
    );

    _dispatchTail[0] = 0xE51FF004; // ldr pc,= address
    _dispatchTail[1] = (u32)exitComboPatchCode->GetExitComboFunction();
    if (_prioDispatch)
    {
        _prioDispatch[0] = 0xE51FF004; // ldr pc,= address
        _prioDispatch[1] = (u32)exitComboPatchCode->GetPrioDispatchFunction();
    }
    _installed = 1;

    // Route the save code through the stub, so it knows when a save is running.
    // These addresses are template values that are only copied once every patch
    // has been applied, so changing them here still reaches the copy.
    if (sdRead)
    {
        u32 readWrapper = (u32)exitComboPatchCode->GetSdReadFunction();
        if (readsave_sdread_asm_address == sdRead)
            readsave_sdread_asm_address = readWrapper;
        if (writesave_sdread_asm_address == sdRead)
            writesave_sdread_asm_address = readWrapper;
        if (verifysave_sdread_asm_address == sdRead)
            verifysave_sdread_asm_address = readWrapper;
    }
    if (sdWrite)
    {
        writesave_sdwrite_asm_address = (u32)exitComboPatchCode->GetSdWriteFunction();
    }

    LOG_DEBUG("Arm7 exit combo enabled\n");
}
