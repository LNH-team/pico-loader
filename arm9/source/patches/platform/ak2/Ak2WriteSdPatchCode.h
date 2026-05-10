#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(ak2_writesd);

extern "C" void ak2_writeSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 ak2_writeSd_sendSdioCommand_address;
extern u32 ak2_writeSd_sdWaitForState_address;
extern u16 ak2_writeSd_sdsc_shift;

class Ak2WriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    Ak2WriteSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode)
        : PatchCode(SECTION_START(ak2_writesd), SECTION_SIZE(ak2_writesd), patchHeap)
    {
        ak2_writeSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        ak2_writeSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)ak2_writeSd);
    }
};
