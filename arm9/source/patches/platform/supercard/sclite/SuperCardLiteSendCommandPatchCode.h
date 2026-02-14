#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(sclite_sd_command_drop);

extern "C" void sclite_sdCommandAndDropResponse6();

class SuperCardLiteSendCommandPatchCode : public PatchCode
{
public:
    explicit SuperCardLiteSendCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_sd_command_drop), SECTION_SIZE(sclite_sd_command_drop), patchHeap) { }

    const void* GetSdCommandAndDropResponse6Function() const
    {
        return GetAddressAtTarget((void*)sclite_sdCommandAndDropResponse6);
    }
};
