#pragma once
#include "sections.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(scsd_sd_command_drop);

extern "C" void scsd_sdCommandAndDropResponse6();

class SuperCardSDSendCommandPatchCode : public PatchCode
{
public:
    explicit SuperCardSDSendCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_sd_command_drop), SECTION_SIZE(scsd_sd_command_drop), patchHeap) { }

    const void* GetSdCommandAndDropResponse6Function() const
    {
        return GetAddressAtTarget((void*)scsd_sdCommandAndDropResponse6);
    }
};
