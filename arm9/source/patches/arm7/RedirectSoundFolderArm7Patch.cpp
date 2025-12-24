#include "common.h"
#include "../PatchContext.h"
#include "RedirectSoundFolderArm7Patch.h"

static const u32 sSoundFolderPathTextPattern[] = { 0x646E616Eu, 0x68732F3Au, 0x64657261u, 0x30252F32u };

bool RedirectSoundFolderArm7Patch::FindPatchTarget(PatchContext& patchContext)
{
    _shared2PathOffset = patchContext.FindPattern32Twl(sSoundFolderPathTextPattern, sizeof(sSoundFolderPathTextPattern));;

    if (_shared2PathOffset)
    {
        LOG_DEBUG("Arm7i shared2 path offset found at 0x%p\n", _shared2PathOffset);
    }
    else
    {
        LOG_WARNING("Arm7i shared2 path offset not found\n");
    }

    return _shared2PathOffset != nullptr;
}

void RedirectSoundFolderArm7Patch::ApplyPatch(PatchContext& patchContext)
{
    if (!_shared2PathOffset)
        return;

    _shared2PathOffset[0] = 0x2F3A6473; // "sd:/"
    _shared2PathOffset[1] = 0x6369705F; // "_pic"
    _shared2PathOffset[2] = 0x6E732F6F; // "o/sn"
    _shared2PathOffset[3] = 0x30252F64; // "d/%0"
}
