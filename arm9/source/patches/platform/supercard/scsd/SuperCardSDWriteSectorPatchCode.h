#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"
#include "../../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(scsd_write_sector);
DEFINE_SECTION_SYMBOLS(scsd_write_data);

extern "C" void scsd_writeSector();
extern "C" void scsd_writeData();

#define INTERWORK_LABEL(function,label) function##_##label##_address

extern u16 scsd_writeSectorSdhcLabel;

extern u32 INTERWORK_LABEL(scsd_writeData, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_changeMode, writeInterwork);
extern u32 INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, writeInterwork);

class SuperCardSDWriteDataPatchCode : public PatchCode
{
public:
    explicit SuperCardSDWriteDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_write_data), SECTION_SIZE(scsd_write_data), patchHeap) { }

    const void* GetWriteDataFunction() const
    {
        return GetAddressAtTarget((void*)scsd_writeData);
    }
};

class SuperCardSDWriteSectorPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    SuperCardSDWriteSectorPatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const ISuperCardSendSdCommandPatchCode* superCardSendSdCommandPatchCode,
        const SuperCardSDWriteDataPatchCode* superCardSDWriteDataPatchCode)
        : PatchCode(SECTION_START(scsd_write_sector), SECTION_SIZE(scsd_write_sector), patchHeap)
    {
        INTERWORK_LABEL(scsd_writeData, writeInterwork) = (u32)superCardSDWriteDataPatchCode->GetWriteDataFunction();
        INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork) = (u32)superCardCommonPatchCode->GetCrc16ChecksumFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
        INTERWORK_LABEL(sccmn_changeMode, writeInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, writeInterwork)
            = (u32)superCardSendSdCommandPatchCode->GetSendSdCommandFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)scsd_writeSector);
    }
};

#undef INTERWORK_LABEL
