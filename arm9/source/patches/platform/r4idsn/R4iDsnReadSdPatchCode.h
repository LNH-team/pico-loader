#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"
#include "../acekard-common/IoRpgSdReadLoopPatchCode.h"

DEFINE_SECTION_SYMBOLS(r4idsn_readsd);

extern "C" void r4idsn_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 r4idsn_readSd_sendSdioCommand_address;
extern u32 r4idsn_readSd_sdReadLoop_address;
extern u32 r4idsn_readSd_sdWaitForState_address;
extern u16 r4idsn_readSd_sdsc_shift;

class R4iDsnReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    R4iDsnReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const IoRpgSdReadLoopPatchCode* iorpgSdReadLoopPatchCode)
        : PatchCode(SECTION_START(r4idsn_readsd), SECTION_SIZE(r4idsn_readsd), patchHeap)
    {
        r4idsn_readSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        r4idsn_readSd_sdReadLoop_address = (u32)iorpgSdReadLoopPatchCode->GetSdReadLoopFunction();
        r4idsn_readSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)r4idsn_readSd);
    }
};
