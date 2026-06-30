#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_read_sector);

extern u16 ez5h_sdhc_read_label;

extern u32 ez5h_readSector_sendSDIOCommand;

extern "C" void ez5h_readSector(u32 sector, void* buffer);

class Ez5hReadSdSectorPatchCode : public PatchCode
{
public:
    Ez5hReadSdSectorPatchCode(PatchHeap& patchHeap,
        const Ez5hSendCommandPatchCode* ez5hSendCommandPatchCode)
        : PatchCode(SECTION_START(ez5h_read_sector), SECTION_SIZE(ez5h_read_sector), patchHeap)
        {
            ez5h_readSector_sendSDIOCommand = (u32)ez5hSendCommandPatchCode->GetSendSDIOCommandFunction();
        }

    const void* GetReadSectorFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_readSector);
    }
};
