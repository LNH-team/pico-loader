#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_sdstoptransmission);

extern "C" void dstt_sdStopTransmission();

class DsttSdStopTransmissionPatchCode : public PatchCode
{
public:
    explicit DsttSdStopTransmissionPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_sdstoptransmission), SECTION_SIZE(dstt_sdstoptransmission), patchHeap) { }

    const void* GetStopTransmissionFunction() const
    {
        return GetAddressAtTarget((void*)dstt_sdStopTransmission);
    }
};
