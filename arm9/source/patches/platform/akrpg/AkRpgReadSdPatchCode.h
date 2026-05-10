#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"
#include "../acekard-common/IoRpgSdReadLoopPatchCode.h"

DEFINE_SECTION_SYMBOLS(akrpg_readsd);

extern "C" void akrpg_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 akrpg_readSd_sendSdioCommand_address;
extern u32 akrpg_readSd_sdReadLoop_address;
extern u32 akrpg_readSd_sdWaitForState_address;
extern u16 akrpg_readSd_sdsc_shift;

class AkRpgReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    AkRpgReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgSdReadLoopPatchCode* iorpgSdReadLoopPatchCode)
        : PatchCode(SECTION_START(akrpg_readsd), SECTION_SIZE(akrpg_readsd), patchHeap)
    {
        akrpg_readSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        akrpg_readSd_sdReadLoop_address = (u32)iorpgSdReadLoopPatchCode->GetSdReadLoopFunction();
        akrpg_readSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)akrpg_readSd);
    }
};
