#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "../acekard-common/IoRpgSendSdioCommandPatchCode.h"
#include "../acekard-common/IoRpgSdWaitForStatePatchCode.h"

DEFINE_SECTION_SYMBOLS(r4idsn_writesd);

extern "C" void r4idsn_writeSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 r4idsn_writeSd_sendSdioCommand_address;
extern u32 r4idsn_writeSd_sdWaitForState_address;
extern u16 r4idsn_writeSd_sdsc_shift;

class R4iDsnWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    explicit R4iDsnWriteSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSendSdioCommandPatchCode* iorpgSendSdioCommandPatchCode,
        const IoRpgSdWaitForStatePatchCode* iorpgSdWaitForStatePatchCode)
        : PatchCode(SECTION_START(r4idsn_writesd), SECTION_SIZE(r4idsn_writesd), patchHeap)
    {
        r4idsn_writeSd_sendSdioCommand_address = (u32)iorpgSendSdioCommandPatchCode->GetSendSdioCommandFunction();
        r4idsn_writeSd_sdWaitForState_address = (u32)iorpgSdWaitForStatePatchCode->GetSDWaitForStateFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)r4idsn_writeSd);
    }
};
