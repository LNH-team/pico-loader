#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "Ak2SdReadSectorPatchCode.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "../acekard-common/IoRpgSdHelperPatchCode.h"

DEFINE_SECTION_SYMBOLS(ak2_readsd);

extern "C" void ak2_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 ak2_readSd_sendSdioCommand_address;
extern u32 ak2_readSd_sdReadSector_address;
extern u32 ak2_readSd_sdWaitForState_address;
extern u16 ak2_readSd_sdsc_shift;

class Ak2ReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    Ak2ReadSdPatchCode(PatchHeap& patchHeap,
        const IoRpgSdHelperPatchCode* iorpgSdHelperPatchCode,
        const Ak2SdReadSectorPatchCode* ak2SdReadSectorPatchCode)
        : PatchCode(SECTION_START(ak2_readsd), SECTION_SIZE(ak2_readsd), patchHeap)
    {
        ak2_readSd_sendSdioCommand_address = (u32)iorpgSdHelperPatchCode->GetSendSdioCommandFunction();
        ak2_readSd_sdReadSector_address = (u32)ak2SdReadSectorPatchCode->GetSDReadSectorFunction();
        ak2_readSd_sdWaitForState_address = (u32)iorpgSdHelperPatchCode->GetSdWaitForStateFunction();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ak2_readSd);
    }
};
