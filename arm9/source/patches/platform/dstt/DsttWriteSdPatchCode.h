#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_writesd);
DEFINE_SECTION_SYMBOLS(dstt_writesd_continue);

extern "C" void dstt_writeSd(u32 dstSector, const void* src, u32 sectorCount);
extern "C" void dstt_writeSdContinue();

extern u32 dstt_writeSdContinue_address;
extern u16 dstt_writeSd_sdsc_shift;

class DsttWriteSdContinuePatchCode : public PatchCode
{
public:
    explicit DsttWriteSdContinuePatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(dstt_writesd_continue), SECTION_SIZE(dstt_writesd_continue), patchHeap) { }

    const void* GetWriteSdContinueFunction() const
    {
        return GetAddressAtTarget((void*)dstt_writeSdContinue);
    }
};

class DsttWriteSdPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    DsttWriteSdPatchCode(PatchHeap& patchHeap,
        const DsttWriteSdContinuePatchCode* dsttWriteSdContinuePatchCode)
        : PatchCode(SECTION_START(dstt_writesd), SECTION_SIZE(dstt_writesd), patchHeap)
    {
        dstt_writeSdContinue_address = (u32)dsttWriteSdContinuePatchCode->GetWriteSdContinueFunction();
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)dstt_writeSd);
    }
};
