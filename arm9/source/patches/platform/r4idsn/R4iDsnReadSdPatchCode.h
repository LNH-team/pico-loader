#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "R4iDsnSdReadSectorPatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "../acekard-common/IoRpgSendSdioCommandPatchCode.h"
#include "../acekard-common/IoRpgSdWaitForStatePatchCode.h"

DEFINE_SECTION_SYMBOLS(r4idsn_readsd);

extern "C" void r4idsn_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 r4idsn_readSd_sendSdioCommand_address;
extern u32 r4idsn_readSd_sdReadSector_address;
extern u32 r4idsn_readSd_sdWaitForState_address;
extern u16 r4idsn_readSd_sdsc_shift;

class R4iDsnReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    R4iDsnReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSendSdioCommandPatchCode* iorpgSendSdioCommandPatchCode,
        const R4iDsnSdReadSectorPatchCode* r4idsnSdReadSectorPatchCode,
        const IoRpgSdWaitForStatePatchCode* iorpgSdWaitForStatePatchCode)
        : PatchCode(SECTION_START(r4idsn_readsd), SECTION_SIZE(r4idsn_readsd), patchHeap)
    {
        r4idsn_readSd_sendSdioCommand_address = (u32)iorpgSendSdioCommandPatchCode->GetSendSdioCommandFunction();
        r4idsn_readSd_sdReadSector_address = (u32)r4idsnSdReadSectorPatchCode->GetSDReadSectorFunction();
        r4idsn_readSd_sdWaitForState_address = (u32)iorpgSdWaitForStatePatchCode->GetSDWaitForStateFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)r4idsn_readSd);
    }
};
