#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"
#include "../../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(sclite_read_sector);
DEFINE_SECTION_SYMBOLS(sclite_read_data);

extern "C" void sclite_readSector();
extern "C" void sclite_readData();

#define INTERWORK_LABEL(function,label) function##_##label##Lite_address

extern u16 sclite_readSectorSdhcLabel;

extern u32 INTERWORK_LABEL(sccmn_changeMode, readInterwork);
extern u32 INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, readInterwork);
extern u32 INTERWORK_LABEL(sclite_readData, readInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork);

class SuperCardLiteReadDataPatchCode : public PatchCode
{
public:
    explicit SuperCardLiteReadDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_read_data), SECTION_SIZE(sclite_read_data), patchHeap) { }

    const void* GetReadDataFunction() const
    {
        return GetAddressAtTarget((void*)sclite_readData);
    }
};

class SuperCardReadSectorLitePatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    SuperCardReadSectorLitePatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const ISuperCardSendSdCommandPatchCode* superCardSendSdCommandPatchCode,
        const SuperCardLiteReadDataPatchCode* superCardLiteReadDataPatchCode)
        : PatchCode(SECTION_START(sclite_read_sector), SECTION_SIZE(sclite_read_sector), patchHeap)
    {
        INTERWORK_LABEL(sccmn_changeMode, readInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, readInterwork)
            = (u32)superCardSendSdCommandPatchCode->GetSendSdCommandFunction();
        INTERWORK_LABEL(sclite_readData, readInterwork) = (u32)superCardLiteReadDataPatchCode->GetReadDataFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)sclite_readSector);
    }
};

#undef INTERWORK_LABEL
