#pragma once
#include "patches/PatchCode.h"
#include "sections.h"
#include "LoaderInfo.h"
#include "patches/platform/IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_pokemondownloader9);

extern "C" void patch_pokemondownloader9_entry(void);

extern const loader_info_t* patch_pokemondownloader9_loader_info_address;
extern u32 patch_pokemondownloader9_readSdSectors_address;

class PokemonDownloaderArm9PatchCode : public PatchCode
{
public:
    PokemonDownloaderArm9PatchCode(PatchHeap& patchHeap, const loader_info_t* loaderInfo,
        const IReadSectorsPatchCode* readSectorsPatchCode)
        : PatchCode(SECTION_START(patch_pokemondownloader9), SECTION_SIZE(patch_pokemondownloader9), patchHeap)
    {
        patch_pokemondownloader9_loader_info_address = loaderInfo;
        patch_pokemondownloader9_readSdSectors_address = (u32)readSectorsPatchCode->GetReadSectorsFunction();
    }

    const void* GetBoot9Function() const
    {
        return GetAddressAtTarget((void*)patch_pokemondownloader9_entry);
    }
};
