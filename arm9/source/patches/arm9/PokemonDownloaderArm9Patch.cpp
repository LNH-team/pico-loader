#include "common.h"
#include "../PatchContext.h"
#include "../platform/LoaderPlatform.h"
#include "PokemonDownloaderArm9PatchCode.h"
#include "PokemonDownloaderArm9Patch.h"

static const u32 sBootFunctionPattern[] = { 0xE92D4010u, 0xE59F003Cu, 0xE5904000u, 0xE3540000u };

bool PokemonDownloaderArm9Patch::FindPatchTarget(PatchContext& patchContext)
{
    _bootFunction = patchContext.FindPattern32(sBootFunctionPattern, sizeof(sBootFunctionPattern));
    return _bootFunction != nullptr;
}

void PokemonDownloaderArm9Patch::ApplyPatch(PatchContext& patchContext)
{
    if (!_bootFunction)
    {
        return;
    }

    auto loaderInfoTarget = (loader_info_t*)patchContext.GetPatchHeap().Alloc(sizeof(loader_info_t));
    memcpy(loaderInfoTarget, _loaderInfo, sizeof(loader_info_t));

    auto sdReadPatchCode = patchContext.GetLoaderPlatform()->CreateSdReadPatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<PokemonDownloaderArm9PatchCode>
    (
        patchContext.GetPatchHeap(),
        loaderInfoTarget,
        sdReadPatchCode
    );

    _bootFunction[0] = 0xE51FF004;
    _bootFunction[1] = (u32)patchCode->GetBoot9Function();
}
