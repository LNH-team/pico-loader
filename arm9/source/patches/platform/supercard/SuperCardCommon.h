#pragma once
#include "sections.h"
#include "thumbInstructions.h"

DEFINE_SECTION_SYMBOLS(scsd_change_mode);
DEFINE_SECTION_SYMBOLS(scsd_common);

extern "C" void sccmn_changeMode();

extern "C" void sccmn_sdSendClock10();
extern "C" void sccmn_sdio4BitCrc16();

class SuperCardChangeModePatchCode : public PatchCode
{
public:
    explicit SuperCardChangeModePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_change_mode), SECTION_SIZE(scsd_change_mode), patchHeap) { }

    const void* GetScChangeModeFunction() const
    {
        return GetAddressAtTarget((void*)sccmn_changeMode);
    }
};

class SuperCardCommonPatchCode : public PatchCode
{
public:
    explicit SuperCardCommonPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_common), SECTION_SIZE(scsd_common), patchHeap) { }

    const void* GetSdSendClock10Function() const
    {
        return GetAddressAtTarget((void*)sccmn_sdSendClock10);
    }

    const void* GetCrc16ChecksumFunction() const
    {
        return GetAddressAtTarget((void*)sccmn_sdio4BitCrc16);
    }
};
