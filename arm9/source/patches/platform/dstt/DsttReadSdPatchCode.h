#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd);
DEFINE_SECTION_SYMBOLS(dstt_readsd_stopTransmission);

extern "C" void dstt_readSd(u32 srcSector, void* dst, u32 sectorCount);
extern "C" void dstt_stopTransmission();

extern u32 dstt_stopTransmission_address;
extern u16 dstt_readSd_sdsc_shift;

class DsttReadSdStopTransmissionPatchCode : public PatchCode
{
public:
    explicit DsttReadSdStopTransmissionPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_readsd_stopTransmission), SECTION_SIZE(dstt_readsd_stopTransmission), patchHeap) { }

    const void* GetStopTransmissionFunction() const
    {
        return GetAddressAtTarget((void*)dstt_stopTransmission);
    }
};

class DsttReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit DsttReadSdPatchCode(PatchHeap& patchHeap,
        const DsttReadSdStopTransmissionPatchCode* dsttReadSdStopTransmissionPatchCode)
        : PatchCode(SECTION_START(dstt_readsd), SECTION_SIZE(dstt_readsd), patchHeap)
        {
            dstt_stopTransmission_address = (u32)dsttReadSdStopTransmissionPatchCode->GetStopTransmissionFunction();
        }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)dstt_readSd);
    }
};
