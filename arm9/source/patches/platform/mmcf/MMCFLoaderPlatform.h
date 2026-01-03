#pragma once
#include "common.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class MMCFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
    void SetCardLocked(bool locked) const override { return; }

    const cf_registers_t& GetCfRegisters() const override
    {
        static constexpr cf_registers_t regs {
            .data           = 0x09000000,
            .altStatus      = 0x088C0000, // should be this, untested, but unused
            .command        = 0x080E0000,
            .error          = 0x08020000,
            .sectorCount    = 0x08040000,
            .lba1           = 0x08060000,
            .lba2           = 0x08080000,
            .lba3           = 0x080A0000,
            .lba4           = 0x080C0000,
        };
        return regs;
    }
};
