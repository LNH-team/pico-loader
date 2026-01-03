#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "../acekard-common/IoRpgSendSdioCommandPatchCode.h"
#include "../acekard-common/IoRpgSdWaitForStatePatchCode.h"

DEFINE_SECTION_SYMBOLS(akrpg_writesd);

extern "C" void akrpg_writeSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 akrpg_writeSd_sendSdioCommand_address;
extern u32 akrpg_writeSd_sdWaitForState_address;
extern u16 akrpg_writeSd_sdsc_shift;

class AkRpgWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    AkRpgWriteSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSendSdioCommandPatchCode* iorpgSendSdioCommandPatchCode,
        const IoRpgSdWaitForStatePatchCode* iorpgSdWaitForStatePatchCode)
        : PatchCode(SECTION_START(akrpg_writesd), SECTION_SIZE(akrpg_writesd), patchHeap)
    {
        akrpg_writeSd_sendSdioCommand_address = (u32)iorpgSendSdioCommandPatchCode->GetSendSdioCommandFunction();
        akrpg_writeSd_sdWaitForState_address = (u32)iorpgSdWaitForStatePatchCode->GetSDWaitForStateFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)akrpg_writeSd);
    }
};
