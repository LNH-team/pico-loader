#pragma once
#include "../PatchCode.h"
#include "sections.h"
#include "LoaderInfo.h"
#include "patches/platform/IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_osresetsystem);
DEFINE_SECTION_SYMBOLS(patch_osresetsystem_boot);

extern "C" void patch_osresetsystem_entry(void);
extern "C" void patch_osresetsystem_bootPicoLoader(void);

extern const loader_info_t* patch_osresetsystem_loader_info_address;
extern u32 patch_osresetsystem_readSdSectors_address;
extern u32 patch_osresetsystem_bootPicoLoader_address;
extern u16 patch_osresetsystem_entry_jump_to_twl_arm7_sync;
extern u32 patch_osresetsystem_arm7Entry_address;

class OSResetSystemPart2PatchCode : public PatchCode
{
public:
    explicit OSResetSystemPart2PatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(patch_osresetsystem_boot), SECTION_SIZE(patch_osresetsystem_boot), patchHeap)
    {
    }

    const void* GetOSResetSystemPart2Function() const
    {
        return GetAddressAtTarget((void*)patch_osresetsystem_bootPicoLoader);
    }
};

class OSResetSystemPatchCode : public PatchCode
{
public:
    OSResetSystemPatchCode(PatchHeap& patchHeap, const loader_info_t* loaderInfo,
        const IReadSectorsPatchCode* readSectorsPatchCode, const OSResetSystemPart2PatchCode* part2PatchCode)
        : PatchCode(SECTION_START(patch_osresetsystem), SECTION_SIZE(patch_osresetsystem), patchHeap)
    {
        patch_osresetsystem_loader_info_address = loaderInfo;
        patch_osresetsystem_readSdSectors_address = (u32)readSectorsPatchCode->GetReadSectorsFunction();
        patch_osresetsystem_bootPicoLoader_address = (u32)part2PatchCode->GetOSResetSystemPart2Function();
    }

    const void* GetOSResetSystemFunction() const
    {
        return GetAddressAtTarget((void*)patch_osresetsystem_entry);
    }
};
