#include "common.h"
#include "patches/PatchContext.h"
#include "LastWindowCrcPatchCode.h"
#include "LastWindowCrcPatch.h"

bool LastWindowCrcPatch::FindPatchTarget(PatchContext& patchContext)
{
    u32* expectedLocation = (u32*)0x020304B4;
    if (*expectedLocation == 0xFAFF4008) // blx SVC_GetCRC16
    {
        _getCrc16 = expectedLocation;
    }

    return true;
}

void LastWindowCrcPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_getCrc16)
    {
        return;
    }

    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<LastWindowCrcPatchCode>(patchContext.GetPatchHeap());
    *_getCrc16 = patchCode->MakeLastWindowCrcBlx((u32)_getCrc16);
}

