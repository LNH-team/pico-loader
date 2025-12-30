#include "common.h"
#include "gameCode.h"
#include "patches/PatchContext.h"
#include "KirbySuperStarUltraPatchAsm.h"
#include "KirbySuperStarUltraPatch.h"

const void* KirbySuperStarUltraPatch::InsertPatch(PatchContext& patchContext)
{
    kirbyultrapatch_nextAddress = next ? (const void*)next->InsertPatch(patchContext) : nullptr;

    switch (patchContext.GetGameCode())
    {
        case GAMECODE("YKWE"):
        {
            kirbyultrapatch_offset = 0x63A8 + 0x160;
            break;
        }
        case GAMECODE("YKWJ"):
        {
            kirbyultrapatch_offset = 0x6300 + 0x160;
            break;
        }
        case GAMECODE("YKWK"):
        {
            kirbyultrapatch_offset = 0x63A8 + 0x160;
            break;
        }
        case GAMECODE("YKWP"):
        {
            kirbyultrapatch_offset = 0x63F0 + 0x160;
            break;
        }
    }

    u32 patchSize = SECTION_SIZE(kirbyultrapatch);
    void* patchAddress = patchContext.GetPatchHeap().Alloc(patchSize);
    u32 entryAddress = (u32)&kirbyultrapatch_entry - (u32)SECTION_START(kirbyultrapatch) + (u32)patchAddress;
    memcpy(patchAddress, SECTION_START(kirbyultrapatch), patchSize);

    return (const void*)entryAddress;
}
