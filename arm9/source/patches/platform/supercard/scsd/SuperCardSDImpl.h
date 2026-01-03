#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../../IReadSectorsPatchCode.h"
#include "../../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(scsd_sd_command_drop);
DEFINE_SECTION_SYMBOLS(scsd_write_sector);
DEFINE_SECTION_SYMBOLS(scsd_read_sector);
DEFINE_SECTION_SYMBOLS(scsd_read_data);
DEFINE_SECTION_SYMBOLS(scsd_write_data);

extern "C" void scsd_sdCommandAndDropResponse6();
extern "C" void scsd_writeSector();
extern "C" void scsd_readSector();
extern "C" void scsd_readData();
extern "C" void scsd_writeData();

#define INTERWORK_LABEL(function,label) function##_##label##_address

extern u16 scsd_readSectorSdhcLabel;
extern u16 scsd_writeSectorSdhcLabel;

extern u32 INTERWORK_LABEL(sccmn_changeMode, readInterwork);
extern u32 INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, readInterwork);
extern u32 INTERWORK_LABEL(scsd_readData, readInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork);

extern u32 INTERWORK_LABEL(scsd_writeData, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_changeMode, writeInterwork);
extern u32 INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, writeInterwork);

class SuperCardSdCommandAndDropPatchCode : public PatchCode
{
public:
    explicit SuperCardSdCommandAndDropPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_sd_command_drop), SECTION_SIZE(scsd_sd_command_drop), patchHeap) { }

    const void* GetSdCommandAndDropResponse6Function() const
    {
        return GetAddressAtTarget((void*)scsd_sdCommandAndDropResponse6);
    }
};

class SuperCardReadDataPatchCode : public PatchCode
{
public:
    explicit SuperCardReadDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_read_data), SECTION_SIZE(scsd_read_data), patchHeap) { }

    const void* GetReadDataFunction() const
    {
        return GetAddressAtTarget((void*)scsd_readData);
    }
};

class SuperCardWriteDataPatchCode : public PatchCode
{
public:
    explicit SuperCardWriteDataPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_write_data), SECTION_SIZE(scsd_write_data), patchHeap) { }

    const void* GetWriteDataFunction() const
    {
        return GetAddressAtTarget((void*)scsd_writeData);
    }
};

class SuperCardWriteSectorPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    SuperCardWriteSectorPatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const SuperCardSdCommandAndDropPatchCode* superCardSdCommandAndDropPatchCode,
        const SuperCardWriteDataPatchCode* superCardWriteDataPatchCode)
        : PatchCode(SECTION_START(scsd_write_sector), SECTION_SIZE(scsd_write_sector), patchHeap)
    {
        INTERWORK_LABEL(scsd_writeData, writeInterwork) = (u32)superCardWriteDataPatchCode->GetWriteDataFunction();
        INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork) = (u32)superCardCommonPatchCode->GetCrc16ChecksumFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork) = (u32)superCardCommonPatchCode->GetSdSendClock10Function();
        INTERWORK_LABEL(sccmn_changeMode, writeInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, writeInterwork)
            = (u32)superCardSdCommandAndDropPatchCode->GetSdCommandAndDropResponse6Function();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)scsd_writeSector);
    }
};

class SuperCardReadSectorPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    SuperCardReadSectorPatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const SuperCardSdCommandAndDropPatchCode* superCardSdCommandAndDropPatchCode,
        const SuperCardReadDataPatchCode* superCardReadDataPatchCode)
        : PatchCode(SECTION_START(scsd_read_sector), SECTION_SIZE(scsd_read_sector), patchHeap)
    {
        INTERWORK_LABEL(sccmn_changeMode, readInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(scsd_sdCommandAndDropResponse6, readInterwork)
            = (u32)superCardSdCommandAndDropPatchCode->GetSdCommandAndDropResponse6Function();
        INTERWORK_LABEL(scsd_readData, readInterwork) = (u32)superCardReadDataPatchCode->GetReadDataFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork) = (u32)superCardCommonPatchCode->GetSdSendClock10Function();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)scsd_readSector);
    }
};

#undef INTERWORK_LABEL
