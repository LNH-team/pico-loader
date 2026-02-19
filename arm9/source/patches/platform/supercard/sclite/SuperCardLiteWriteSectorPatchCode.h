#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"
#include "../../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(sclite_write_sector);
DEFINE_SECTION_SYMBOLS(sclite_write_data);

extern "C" void sclite_writeSector();
extern "C" void sclite_writeData();

#define INTERWORK_LABEL(function,label) function##_##label##Lite_address

extern u16 sclite_writeSectorSdhcLabel;

extern u32 INTERWORK_LABEL(sclite_writeData, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_changeMode, writeInterwork);
extern u32 INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, writeInterwork);

class SuperCardLiteWriteDataPatchCode : public PatchCode
{
public:
    explicit SuperCardLiteWriteDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_write_data), SECTION_SIZE(sclite_write_data), patchHeap) { }

    const void* GetWriteDataFunction() const
    {
        return GetAddressAtTarget((void*)sclite_writeData);
    }
};

class SuperCardLiteWriteSectorPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    SuperCardLiteWriteSectorPatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const ISuperCardSendSdCommandPatchCode* superCardSendSdCommandPatchCode,
        const SuperCardLiteWriteDataPatchCode* superCardLiteWriteDataPatchCode)
        : PatchCode(SECTION_START(sclite_write_sector), SECTION_SIZE(sclite_write_sector), patchHeap)
    {
        INTERWORK_LABEL(sclite_writeData, writeInterwork) = (u32)superCardLiteWriteDataPatchCode->GetWriteDataFunction();
        INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork) = (u32)superCardCommonPatchCode->GetCrc16ChecksumFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
        INTERWORK_LABEL(sccmn_changeMode, writeInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, writeInterwork)
            = (u32)superCardSendSdCommandPatchCode->GetSendSdCommandFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)sclite_writeSector);
    }
};

#undef INTERWORK_LABEL
