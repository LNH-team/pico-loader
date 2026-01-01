#pragma once
#include "common.h"
#include "M3CFCommon.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class M3CFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
public:
    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto lockUnlock = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new M3CFChangeModePatchCode(patchHeap);
        });
        return CreateCommonCfReadPatchCode(patchCodeCollection, patchHeap, lockUnlock->GetM3LockUnlockCardFunction());
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto lockUnlock = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new M3CFChangeModePatchCode(patchHeap);
        });
        return CreateCommonCfWritePatchCode(patchCodeCollection, patchHeap, lockUnlock->GetM3LockUnlockCardFunction());
    }
    
private:
    bool RequiresLocking() const override { return true; }

    void CardUnlock() const override;

    void CardLock() const override;

    const CompactFlash::CF_REGISTERS& GetCfRegisters() const override
    {
        static constexpr CompactFlash::CF_REGISTERS regs {
            .data           = 0x08800000,
            .status         = 0x080C0000,
            .command        = 0x088E0000,
            .error          = 0x08820000,
            .sectorCount    = 0x08840000,
            .lba1           = 0x08860000,
            .lba2           = 0x08880000,
            .lba3           = 0x088A0000,
            .lba4           = 0x088C0000,
        };
        return regs;
    }
};
