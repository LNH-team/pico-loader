#pragma once
#include "sections.h"
#include "../IReadSectorsPatchCode.h"
#include "Ez5hDoSdOperationPatchCode.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_read_multiple_sector);

extern u16 ez5h_sdhc_read_label;
extern u32 ez5h_readMultipleSector_doSDOperation;

extern "C" void ez5h_readMultipleSector(u32 sector, void* buffer);

class Ez5hReadMultipleSdSectorPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    Ez5hReadMultipleSdSectorPatchCode(PatchHeap& patchHeap,
		const Ez5hDoSdOperationPatchCode* ez5hDoSdOperationPatchCode)
        : PatchCode(SECTION_START(ez5h_read_multiple_sector), SECTION_SIZE(ez5h_read_multiple_sector), patchHeap)
        {
			ez5h_readMultipleSector_doSDOperation = (u32)ez5hDoSdOperationPatchCode->GetDoSDOperationFunction();
        }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)ez5h_readMultipleSector);
    }
};
