#pragma once
#include "sections.h"
#include "../SuperCardCommon.h"
#include "patches/PatchCode.h"
#include "../../IReadSectorsPatchCode.h"
#include "../../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(sclite_sd_command_drop);
DEFINE_SECTION_SYMBOLS(sclite_write_sector);
DEFINE_SECTION_SYMBOLS(sclite_read_sector);
DEFINE_SECTION_SYMBOLS(sclite_read_data);
DEFINE_SECTION_SYMBOLS(sclite_write_data);

extern "C" void sclite_sdCommandAndDropResponse6();
extern "C" void sclite_writeSector();
extern "C" void sclite_readSector();
extern "C" void sclite_readData();
extern "C" void sclite_writeData();

#define INTERWORK_LABEL(function,label) function##_##label##Lite_address

extern u16 sclite_readSectorSdhcLabel;
extern u16 sclite_writeSectorSdhcLabel;

extern u32 INTERWORK_LABEL(sccmn_changeMode, readInterwork);
extern u32 INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, readInterwork);
extern u32 INTERWORK_LABEL(sclite_readData, readInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork);

extern u32 INTERWORK_LABEL(sclite_writeData, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork);
extern u32 INTERWORK_LABEL(sccmn_changeMode, writeInterwork);
extern u32 INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, writeInterwork);

class SuperCardSDCommandAndDropLitePatchCode : public PatchCode
{
public:
    explicit SuperCardSDCommandAndDropLitePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_sd_command_drop), SECTION_SIZE(sclite_sd_command_drop), patchHeap) { }

    const void* GetSdCommandAndDropResponse6Function() const
    {
        return GetAddressAtTarget((void*)sclite_sdCommandAndDropResponse6);
    }
};

class SuperCardReadDataLitePatchCode : public PatchCode
{
public:
    explicit SuperCardReadDataLitePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_read_data), SECTION_SIZE(sclite_read_data), patchHeap) { }

    const void* GetReadDataLiteFunction() const
    {
        return GetAddressAtTarget((void*)sclite_readData);
    }
};

class SuperCardWriteDataLitePatchCode : public PatchCode
{
public:
    explicit SuperCardWriteDataLitePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_write_data), SECTION_SIZE(sclite_write_data), patchHeap) { }

    const void* GetWriteDataLiteFunction() const
    {
        return GetAddressAtTarget((void*)sclite_writeData);
    }
};

class SuperCardReadSectorLitePatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    SuperCardReadSectorLitePatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const SuperCardSDCommandAndDropLitePatchCode* superCardSdCommandAndDropLitePatchCode,
        const SuperCardReadDataLitePatchCode* superCardReadDataLitePatchCode)
        : PatchCode(SECTION_START(sclite_read_sector), SECTION_SIZE(sclite_read_sector), patchHeap)
    {
        INTERWORK_LABEL(sccmn_changeMode, readInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, readInterwork)
            = (u32)superCardSdCommandAndDropLitePatchCode->GetSdCommandAndDropResponse6Function();
        INTERWORK_LABEL(sclite_readData, readInterwork) = (u32)superCardReadDataLitePatchCode->GetReadDataLiteFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, readInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)sclite_readSector);
    }
};

class SuperCardWriteSectorLitePatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    SuperCardWriteSectorLitePatchCode(PatchHeap& patchHeap,
        const SuperCardCommonPatchCode* superCardCommonPatchCode,
        const SuperCardChangeModePatchCode* superCardChangeModePatchCode,
        const SuperCardSDCommandAndDropLitePatchCode* superCardSdCommandAndDropLitePatchCode,
        const SuperCardWriteDataLitePatchCode* superCardWriteDataLitePatchCode)
        : PatchCode(SECTION_START(sclite_write_sector), SECTION_SIZE(sclite_write_sector), patchHeap)
    {
        INTERWORK_LABEL(sclite_writeData, writeInterwork) = (u32)superCardWriteDataLitePatchCode->GetWriteDataLiteFunction();
        INTERWORK_LABEL(sccmn_sdio4BitCrc16, writeInterwork) = (u32)superCardCommonPatchCode->GetCrc16ChecksumFunction();
        INTERWORK_LABEL(sccmn_sdSendClock10, writeInterwork) = (u32)superCardChangeModePatchCode->GetSdSendClock10Function();
        INTERWORK_LABEL(sccmn_changeMode, writeInterwork) = (u32)superCardChangeModePatchCode->GetScChangeModeFunction();
        INTERWORK_LABEL(sclite_sdCommandAndDropResponse6, writeInterwork)
            = (u32)superCardSdCommandAndDropLitePatchCode->GetSdCommandAndDropResponse6Function();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)sclite_writeSector);
    }
};

#undef INTERWORK_LABEL
