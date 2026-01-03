#pragma once
#include "../PatchCode.h"
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_cardireadromidcore);

extern "C" u32 patch_cardireadromidcore_entry(void);

extern u32 patch_cardireadromidcore_cardid_address;

class CardiReadRomIdCorePatchCode : public PatchCode
{
public:
    CardiReadRomIdCorePatchCode(PatchHeap& patchHeap, const void* cardIdPointer)
        : PatchCode(SECTION_START(patch_cardireadromidcore), SECTION_SIZE(patch_cardireadromidcore), patchHeap)
    {
        patch_cardireadromidcore_cardid_address = (u32)cardIdPointer;
    }

    const void* GetCardiReadRomIdCoreFunction() const
    {
        return GetAddressAtTarget((void*)patch_cardireadromidcore_entry);
    }
};
