#pragma once
#include "sections.h"
#include "patches/PatchCode.h"
#include "../IWriteSectorsPatchCode.h"

DEFINE_SECTION_SYMBOLS(m3ds_writesdsectors);
DEFINE_SECTION_SYMBOLS(m3ds_writesdsectors_sendSector);

extern "C" void m3ds_writeSdSectors(u32 dstSector, const void* src, u32 sectorCount);
extern "C" void m3ds_sendSector(const void* src);

extern u32 m3ds_sendSector_address;

class M3DSSendSectorPatchCode : public PatchCode
{
public:
    explicit M3DSSendSectorPatchCode(PatchHeap& patchHeap)
        : PatchCode(SECTION_START(m3ds_writesdsectors_sendSector), SECTION_SIZE(m3ds_writesdsectors_sendSector), patchHeap) { }

    const void* GetSendSectorFunction() const
    {
        return GetAddressAtTarget((void*)m3ds_sendSector);
    }
};

class M3DSWriteSdSectorsPatchCode : public PatchCode, public IWriteSectorsPatchCode
{
public:
    M3DSWriteSdSectorsPatchCode(PatchHeap& patchHeap, const M3DSSendSectorPatchCode* m3dsSendSectorPatchCode)
        : PatchCode(SECTION_START(m3ds_writesdsectors), SECTION_SIZE(m3ds_writesdsectors), patchHeap)
        {
            m3ds_sendSector_address = (u32)m3dsSendSectorPatchCode->GetSendSectorFunction();
        }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)m3ds_writeSdSectors);
    }
};
