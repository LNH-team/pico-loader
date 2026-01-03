#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "patches/platform/IReadSectorsPatchCode.h"
#include "patches/platform/IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_dsisdredirect);

extern "C" bool __patch_dsisdredirect_io(u32 driveNumber, u32 startSector, void* buffer, u32 sectorCount, bool isRead);
extern "C" u32 __patch_dsisdredirect_control(u32 driveNumber, u32 command, void* argumentBuffer);

extern u32 __patch_dsisdredirect_io_readsd_asm_address;
extern u32 __patch_dsisdredirect_io_writesd_asm_address;
extern u32 __patch_dsisdredirect_control_get_drive_struct_address;

class Sdk5DsiSdCardRedirectPatchCode : public PatchCode
{
public:
    Sdk5DsiSdCardRedirectPatchCode(PatchHeap& patchHeap, const IReadSectorsPatchCode* readSectorsPatchCode,
        const IWriteSectorsPatchCode* writeSectorsPatchCode, u32 getDriveStructAddress)
        : PatchCode(SECTION_START(patch_dsisdredirect), SECTION_SIZE(patch_dsisdredirect), patchHeap)
    {
        __patch_dsisdredirect_io_readsd_asm_address = (u32)readSectorsPatchCode->GetReadSectorsFunction();
        __patch_dsisdredirect_io_writesd_asm_address =  (u32)writeSectorsPatchCode->GetWriteSectorFunction();
        __patch_dsisdredirect_control_get_drive_struct_address = getDriveStructAddress;
    }

    const void* GetIoFunction() const
    {
        return GetAddressAtTarget((void*)__patch_dsisdredirect_io);
    }

    const void* GetControlFunction() const
    {
        return GetAddressAtTarget((void*)__patch_dsisdredirect_control);
    }
};