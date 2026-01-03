#pragma once
#include "common.h"
#include "M3CFLockUnlockCard.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class M3CFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
    bool RequiresLocking() const override { return true; }
    
    CompactFlashLockUnlockPatchCode* NewCardLockUnlockPatchCode(PatchHeap& patchHeap) const override
    {
        return new M3CFLockUnlockCardPatchCode(patchHeap);
    }

    void SetCardLocked(bool locked) const override;

    const cf_registers_t& GetCfRegisters() const override
    {
        static constexpr cf_registers_t regs {
            .data           = 0x08800000,
            .altStatus      = 0x080C0000,
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
