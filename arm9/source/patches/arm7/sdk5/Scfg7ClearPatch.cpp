#include "common.h"
#include "thumbInstructions.h"
#include "Scfg7ClearPatch.h"

// patch target is inside the end of the sdk5 function that clears the scfg7 enabled bit
static const u32 sPattern[] = { 0xE5810000u, 0xE12FFF1Eu, 0x04004008u, 0x04004020u };

bool Scfg7ClearPatch::FindPatchTarget(PatchContext& patchContext)
{
    _patchLocation = patchContext.FindPattern32(sPattern, sizeof(sPattern));

    if (_patchLocation != nullptr)
    {
        LOG_DEBUG("Scfg7 Clear patch: %p\n", _patchLocation);
    }

    return _patchLocation != nullptr;
}

void Scfg7ClearPatch::ApplyPatch(PatchContext& patchContext)
{
    if (_patchLocation == nullptr)
    {
        return;
    }

    _patchLocation[0] = 0xE1A00000;
}
