#pragma once
#include "common.h"
#include "SuperCardCFLockUnlockCard.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class SuperCardCFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
    bool RequiresLocking() const override { return true; }
    
    CompactFlashLockUnlockPatchCode* NewCardLockUnlockPatchCode(PatchHeap& patchHeap) const    override
    {
        return new SuperCardCFLockUnlockCardPatchCode(patchHeap);
    }

    void CardLockUnlock(bool lock) const override;

    const cf_registers_t& GetCfRegisters() const override
    {
        static constexpr cf_registers_t regs {
            .data           = 0x09000000,
            .altStatus      = 0x098C0000,
            .command        = 0x090E0000,
            .error          = 0x09020000,
            .sectorCount    = 0x09040000,
            .lba1           = 0x09060000,
            .lba2           = 0x09080000,
            .lba3           = 0x090A0000,
            .lba4           = 0x090C0000,
        };
        return regs;
    }
};
