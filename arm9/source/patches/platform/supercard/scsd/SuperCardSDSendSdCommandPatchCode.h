#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"

DEFINE_SECTION_SYMBOLS(scsd_sd_command_drop);

extern "C" void scsd_sdCommandAndDropResponse6();

class SuperCardSDSendSdCommandPatchCode : public PatchCode, public ISuperCardSendSdCommandPatchCode
{
public:
    explicit SuperCardSDSendSdCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(scsd_sd_command_drop), SECTION_SIZE(scsd_sd_command_drop), patchHeap) { }

    const void* GetSendSdCommandFunction() const override
    {
        return GetAddressAtTarget((void*)scsd_sdCommandAndDropResponse6);
    }
};
