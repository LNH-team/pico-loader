#include "common.h"
#include "gameCode.h"
#include "patches/PatchContext.h"
#include "RabbidsGoHomePatchAsm.h"
#include "RabbidsGoHomePatch.h"

const void* RabbidsGoHomePatch::InsertPatch(PatchContext& patchContext)
{
    rabbidspatch_nextAddress = next ? (const void*)next->InsertPatch(patchContext) : nullptr;

    switch (patchContext.GetGameCode())
    {
        case GAMECODE("VRGE"):
        {
            rabbidspatch_offset = 0x1465C + 0x1C;
            break;
        }
        case GAMECODE("VRGV"):
        {
            rabbidspatch_offset = 0x146A4 + 0x1C;
            break;
        }
    }

    u32 patchSize = SECTION_SIZE(rabbidspatch);
    void* patchAddress = patchContext.GetPatchHeap().Alloc(patchSize);
    u32 entryAddress = (u32)&rabbidspatch_entry - (u32)SECTION_START(rabbidspatch) + (u32)patchAddress;
    memcpy(patchAddress, SECTION_START(rabbidspatch), patchSize);

    return (const void*)entryAddress;
}
