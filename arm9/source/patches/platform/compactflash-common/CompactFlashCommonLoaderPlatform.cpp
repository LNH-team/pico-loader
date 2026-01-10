#include "common.h"
#include <libtwl/mem/memExtern.h>
#include "CompactFlashCommonLoaderPlatform.h"

static constexpr int CF_CARD_TIMEOUT = 10000000;
static constexpr int CF_STS_READY = 0x40;
static constexpr int CF_STS_DSC = 0x10;
static constexpr int CF_STS_BUSY = 0x80;

bool CompactFlashCommonLoaderPlatform::InitializeSdCard()
{
    u32 oldMemCnt = REG_EXMEMCNT;
    mem_setGbaCartridgeRomWaits(EXMEMCNT_SLOT2_ROM_WAIT1_10, EXMEMCNT_SLOT2_ROM_WAIT2_6);
    SetCardLocked(false);
    auto res = InitializeCfCard();
    SetCardLocked(true);
    REG_EXMEMCNT = oldMemCnt;
    return res;
}

static bool waitAvailableForCommands(const cf_registers_t& registers)
{
    // wait for card to finish previous commands
    for (int i = 0; i < CF_CARD_TIMEOUT; i++)
    {
        if ((*registers.command & CF_STS_BUSY) == 0)
        {
            break;
        }
    }

    // wait for card to be ready for new commands
    for (int i = 0; i < CF_CARD_TIMEOUT; i++)
    {
        if ((*registers.command & (CF_STS_READY | CF_STS_DSC)) != 0)
        {
            return true;
        }
    }

    return false;
}

bool CompactFlashCommonLoaderPlatform::InitializeCfCard()
{
    const auto& registers = GetCfRegisters();
    if (!waitAvailableForCommands(registers))
    {
        return false;
    }
    // Check that the registers are writable and hold the values we set
    u16 temp = *registers.lba1;
    *registers.lba1 = (~temp & 0xFF);
    temp = (~temp & 0xFF);
    if (*registers.lba1 != temp)
    {
        return false;
    }

    *registers.lba1 = 0xAA55;
    if (*registers.lba1 == 0xAA55)
    {
        return false;
    }
    return true;
}
