#include "common.h"
#include "patches/PatchContext.h"
#include "LastWindowCrcPatchCode.h"
#include "LastWindowCrcPatch.h"

bool LastWindowCrcPatch::FindPatchTarget(PatchContext& patchContext)
{
    _getCrc16 = (u32*)0x020304B4;
    return true;
}

void LastWindowCrcPatch::ApplyPatch(PatchContext& patchContext)
{
    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<LastWindowCrcPatchCode>(patchContext.GetPatchHeap());
    u32 patchAddr = (u32)patchCode->GetLastWindowCrcFunction();
    *_getCrc16 = MakeBlxCall(patchAddr);
}

