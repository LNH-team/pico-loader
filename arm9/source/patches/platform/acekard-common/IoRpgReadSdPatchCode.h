#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "IoRpgSdHelperPatchCode.h"
#include "IoRpgSdReadLoopPatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_readsd);

extern "C" void iorpg_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 iorpg_readSd_sendSdioCommand_address;
extern u32 iorpg_readSd_sdReadLoop_address;
extern u32 iorpg_readSd_sdWaitForState_address;
extern u32 iorpg_readSd_cmd12_command;
extern u32 iorpg_readSd_cmd18_command;
extern u16 iorpg_readSd_sdsc_shift;

class IoRpgReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    IoRpgReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgSdReadLoopPatchCode* iorpgSdReadLoopPatchCode,
        const IoRpgPlatformSpecifics& platformSpecifics)
        : PatchCode(SECTION_START(iorpg_readsd), SECTION_SIZE(iorpg_readsd), patchHeap)
    {
        iorpg_readSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        iorpg_readSd_sdReadLoop_address = (u32)iorpgSdReadLoopPatchCode->GetSdReadLoopFunction();
        iorpg_readSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
        iorpg_readSd_cmd12_command = platformSpecifics.cmd12Command;
        iorpg_readSd_cmd18_command = platformSpecifics.cmd18Command;
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)iorpg_readSd);
    }
};
