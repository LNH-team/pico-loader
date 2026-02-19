#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../ISuperCardSendSdCommandPatchCode.h"

DEFINE_SECTION_SYMBOLS(sclite_sd_command_drop);

extern "C" void sclite_sdCommandAndDropResponse6();

class SuperCardLiteSendSdCommandPatchCode : public PatchCode, public ISuperCardSendSdCommandPatchCode
{
public:
    explicit SuperCardLiteSendSdCommandPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(sclite_sd_command_drop), SECTION_SIZE(sclite_sd_command_drop), patchHeap) { }

    const void* GetSendSdCommandFunction() const override
    {
        return GetAddressAtTarget((void*)sclite_sdCommandAndDropResponse6);
    }
};
