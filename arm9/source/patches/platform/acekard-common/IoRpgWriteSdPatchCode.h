#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "IoRpgSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_writesd);

extern "C" void iorpg_writeSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 iorpg_writeSd_sendSdioCommand_address;
extern u32 iorpg_writeSd_sdWaitForState_address;
extern u32 iorpg_writeSd_cmd24_command;
extern u16 iorpg_writeSd_sdsc_shift;

class IoRpgWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    IoRpgWriteSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgPlatformSpecifics& platformSpecifics)
        : PatchCode(SECTION_START(iorpg_writesd), SECTION_SIZE(iorpg_writesd), patchHeap)
    {
        iorpg_writeSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        iorpg_writeSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
        iorpg_writeSd_cmd24_command = platformSpecifics.cmd24Command;
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)iorpg_writeSd);
    }
};
