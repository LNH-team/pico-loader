#pragma once
#include "common.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class MMCFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
protected:
    void SetCardLocked(bool locked) const override { }

    const cf_registers_t& GetCfRegisters() const override
    {
        static const cf_registers_t registers
        {
            .data           = (vu16*)0x09000000,
            .altStatus      = (vu16*)0x088C0000, // should be this, untested, but unused
            .command        = (vu16*)0x080E0000,
            .error          = (vu16*)0x08020000,
            .sectorCount    = (vu16*)0x08040000,
            .lba1           = (vu16*)0x08060000,
            .lba2           = (vu16*)0x08080000,
            .lba3           = (vu16*)0x080A0000,
            .lba4           = (vu16*)0x080C0000,
        };
        return registers;
    }
};
