#pragma once
#include "common.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class MMCFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
    const CompactFlash::CF_REGISTERS& GetCfRegisters() const override
    {
        static constexpr CompactFlash::CF_REGISTERS regs {
            .data           = 0x09000000,
            .status         = 0x080E0000, // dldi and official sf had it same as `command` register, but should it be 0x080C0000?
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
