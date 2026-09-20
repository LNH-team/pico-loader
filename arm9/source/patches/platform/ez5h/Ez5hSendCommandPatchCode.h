#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_send_command);

extern "C" u8 ez5h_sendCommand(u32 byteswapped_low, u32 non_byteswapped_high);
extern "C" u64 ez5h_sendSDIOCommand(u8 command, u32 parameter);

class Ez5hSendCommandPatchCode : public PatchCode
{
public:
    Ez5hSendCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(ez5h_send_command), SECTION_SIZE(ez5h_send_command), patchHeap)
    {
    }

    const void* GetSendCommandFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_sendCommand);
    }

    const void* GetSendSDIOCommandFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_sendSDIOCommand);
    }
};
