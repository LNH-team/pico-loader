#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"
#include "../acekard-common/IoRpgSdReadLoopPatchCode.h"

DEFINE_SECTION_SYMBOLS(ak2_readsd);

extern "C" void ak2_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 ak2_readSd_sendSdioCommand_address;
extern u32 ak2_readSd_sdReadLoop_address;
extern u32 ak2_readSd_sdWaitForState_address;
extern u16 ak2_readSd_sdsc_shift;

class Ak2ReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    Ak2ReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgSdReadLoopPatchCode* iorpgSdReadLoopPatchCode)
        : PatchCode(SECTION_START(ak2_readsd), SECTION_SIZE(ak2_readsd), patchHeap)
    {
        ak2_readSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        ak2_readSd_sdReadLoop_address = (u32)iorpgSdReadLoopPatchCode->GetSdReadLoopFunction();
        ak2_readSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ak2_readSd);
    }
};
