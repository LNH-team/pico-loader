#include "common.h"
#include "patches/PatchContext.h"
#include "thumbInstructions.h"
#include "CardiReadRomWithDmaPatch.h"
#include "CardiIsRomDmaAvailablePatch.h"

static const u32 sCARDiIsRomDmaAvailablePattern[] = { 0xE92D43F8u, 0xE3A04000u, 0xE1A09001u, 0xE1A08002u };
static const u32 sCARDiIsRomDmaAvailablePatternThumb[] = { 0xB082B5F8u, 0x1C0D2400u, 0x1C1E9200u, 0x94011C21u };
static const u32 sCARDiIsRomDmaAvailablePatternThumbHybrid[] = { 0xB082B5F8u, 0x21001C0Du, 0x92009101u, 0x27001C1Cu };

static const u32 sReturnFalsePatchArm[] = { 0xE3A00000, 0xE12FFF1E }; // mov r0, #0; bx lr

bool CardiIsRomDmaAvailablePatch::FindPatchTarget(PatchContext& patchContext)
{
    _cardiIsRomDmaAvailable = patchContext.FindPattern32(sCARDiIsRomDmaAvailablePattern, sizeof(sCARDiIsRomDmaAvailablePattern));
    if (!_cardiIsRomDmaAvailable)
    {
        _cardiIsRomDmaAvailable = patchContext.FindPattern32(sCARDiIsRomDmaAvailablePatternThumbHybrid, sizeof(sCARDiIsRomDmaAvailablePatternThumbHybrid));
        if (_cardiIsRomDmaAvailable)
        {
            _thumb = true;
        }
        else
        {
            _cardiIsRomDmaAvailable = patchContext.FindPattern32(sCARDiIsRomDmaAvailablePatternThumb, sizeof(sCARDiIsRomDmaAvailablePatternThumb));
            if (_cardiIsRomDmaAvailable)
            {
                _thumb = true;
            }
        }
    }
    if (!_cardiIsRomDmaAvailable)
    {
        LOG_WARNING("CARDi_IsRomDmaAvailable not found\n");
    }

    // If an application uses no rom reads, the function is not included in the rom.
    return true; //_cardiIsRomDmaAvailable != nullptr;
}

void CardiIsRomDmaAvailablePatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiIsRomDmaAvailable)
        return;
    if (_cardiReadRomWithDmaPatch && _cardiReadRomWithDmaPatch->WillApply())
        return;
    if (_thumb)
    {
        ((u16*)_cardiIsRomDmaAvailable)[0] = THUMB_MOVS_IMM(THUMB_R0, 0);
        ((u16*)_cardiIsRomDmaAvailable)[1] = THUMB_BX_LR;
    }
    else
    {
        _cardiIsRomDmaAvailable[0] = sReturnFalsePatchArm[0];
        _cardiIsRomDmaAvailable[1] = sReturnFalsePatchArm[1];
    }
}
