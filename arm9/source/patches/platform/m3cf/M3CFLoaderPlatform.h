#pragma once
#include "common.h"
#include "M3CFLockUnlockCardPatchCode.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class M3CFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
protected:
    const ICompactFlashLockUnlockPatchCode* CreateLockingPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new M3CFLockUnlockCardPatchCode(patchHeap);
        });
    }

    void SetCardLocked(bool locked) const override;

    const cf_registers_t& GetCfRegisters() const override
    {
        static const cf_registers_t registers
        {
            .data           = (vu16*)0x08800000,
            .altStatus      = (vu16*)0x080C0000,
            .command        = (vu16*)0x088E0000,
            .error          = (vu16*)0x08820000,
            .sectorCount    = (vu16*)0x08840000,
            .lba1           = (vu16*)0x08860000,
            .lba2           = (vu16*)0x08880000,
            .lba3           = (vu16*)0x088A0000,
            .lba4           = (vu16*)0x088C0000,
        };
        return registers;
    }
};
