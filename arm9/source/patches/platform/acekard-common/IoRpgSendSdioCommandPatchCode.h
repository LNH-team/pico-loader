#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(iorpg_sendsdiocommand);

extern "C" void iorpg_sendSdioCommand(u8 sdio, u8 param_type, u32 param, u32 read_len);

class IoRpgSendSdioCommandPatchCode : public PatchCode
{
public:
    explicit IoRpgSendSdioCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(iorpg_sendsdiocommand), SECTION_SIZE(iorpg_sendsdiocommand), patchHeap) { }

    const void* GetSendSdioCommandFunction() const
    {
        return GetAddressAtTarget((void*)iorpg_sendSdioCommand);
    }
};
