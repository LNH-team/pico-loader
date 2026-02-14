#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"
#include "../../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(scsd_read_sector);
DEFINE_SECTION_SYMBOLS(scsd_read_data);

extern "C" void scsd_readSector();
extern "C" void scsd_readData();

#define INTERWORK_LABEL(function,label) function##_##label##_address

extern u16 scsd_readSectorSdhcLabel;

extern u32 INTERWORK_LABEL(sccmn_changeMode, readInterwork);
extern u32 INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, readInterwork);
extern u32 INTERWORK_LABEL(scsd_readData, readInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork);

class SuperCardSDReadDataPatchCode : public PatchCode
{
public:
    explicit SuperCardSDReadDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_read_data), SECTION_SIZE(scsd_read_data), patchHeap) { }

    const void* GetReadDataFunction() const
    {
        return GetAddressAtTarget((void*)scsd_readData);
    }
};

class SuperCardSDReadSectorPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    SuperCardSDReadSectorPatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const ISuperCardSendSdCommandPatchCode* superCardSendSdCommandPatchCode,
        const SuperCardSDReadDataPatchCode* superCardSDReadDataPatchCode)
        : PatchCode(SECTION_START(scsd_read_sector), SECTION_SIZE(scsd_read_sector), patchHeap)
    {
        INTERWORK_LABEL(sccmn_changeMode, readInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, readInterwork)
            = (u32)superCardSendSdCommandPatchCode->GetSendSdCommandFunction();
        INTERWORK_LABEL(scsd_readData, readInterwork) = (u32)superCardSDReadDataPatchCode->GetReadDataFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)scsd_readSector);
    }
};

#undef INTERWORK_LABEL
