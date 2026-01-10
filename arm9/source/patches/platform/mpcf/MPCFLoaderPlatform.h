#pragma once
#include "common.h"
#include "../compactflash-common/CompactFlashCommonLoaderPlatform.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class MPCFLoaderPlatform : public CompactFlashCommonLoaderPlatform
{
protected:
    void SetCardLocked(bool locked) const override { }

    const cf_registers_t& GetCfRegisters() const override
    {
        static const cf_registers_t registers
        {
            .data           = (vu16*)0x09000000,
            .altStatus      = (vu16*)0x098C0000,
            .command        = (vu16*)0x090E0000,
            .error          = (vu16*)0x09020000,
            .sectorCount    = (vu16*)0x09040000,
            .lba1           = (vu16*)0x09060000,
            .lba2           = (vu16*)0x09080000,
            .lba3           = (vu16*)0x090A0000,
            .lba4           = (vu16*)0x090C0000,
        };
        return registers;
    }
};
