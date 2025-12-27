#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "IsNitroSdReadAsm.h"
#include "IsNitroSdWriteAsm.h"
#include "sharedMemory.h"

/// @brief Implementation of LoaderPlatform for the IS-NITRO-EMULATOR with agb semihosting
class IsNitroLoaderPlatform : public LoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new IsNitroSdReadPatchCode(patchHeap, GetAgbRamPtr());
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new IsNitroSdWritePatchCode(patchHeap, GetAgbRamPtr());
        });
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

private:
    u32 GetAgbRamPtr() const
    {
        return *(vu32*)&TWL_SHARED_MEMORY->ntrSharedMem.isDebuggerData[0x1C];
    }
};
