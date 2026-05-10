#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_sdhelper);

extern "C" void iorpg_sendSdioCommand(u8 sdio, u8 param_type, u32 param, u32 read_len);
extern "C" void iorpg_sdWaitForState(u32 status_shift, u8 state);

class IoRpgSdHelperPatchCode : public PatchCode
{
public:
    IoRpgSdHelperPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(iorpg_sdhelper), SECTION_SIZE(iorpg_sdhelper), patchHeap) { }

    const void* GetSendSdioCommandFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_sendSdioCommand);
    }

    const void* GetSdWaitForStateFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_sdWaitForState);
    }
};
