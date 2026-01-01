#include "common.h"
#include <libtwl/mem/memExtern.h>
#include "thumbInstructions.h"
#include "CompactFlashCommonLoaderPlatform.h"

static constexpr int CF_CARD_TIMEOUT = 10000000;
static constexpr int CF_STS_INSERTED = 0x50;
static constexpr int CF_STS_BUSY = 0x80;

bool CompactFlashCommonLoaderPlatform::InitializeSdCard()
{
    u32 oldMemCnt = REG_EXMEMCNT;
    mem_setGbaCartridgeCpu(EXMEMCNT_SLOT2_CPU_ARM9);
    mem_setGbaCartridgeRomWaits(EXMEMCNT_SLOT2_ROM_WAIT1_10, EXMEMCNT_SLOT2_ROM_WAIT2_6);
    CardUnlock();
    auto res = InitializeCFCard();
    CardLock();
    REG_EXMEMCNT = oldMemCnt;
    mem_setGbaCartridgeCpu(EXMEMCNT_SLOT2_CPU_ARM7);
    return res;
}

static bool waitAvailableForCommands(const auto& regs) {
    auto commandReg = (vu16*)regs.command;
    auto statusReg = (vu16*)regs.status;
    // wait for card to finish previous commands
    for (int i = 0; i < CF_CARD_TIMEOUT; i++)
    {
        if ((*commandReg & CF_STS_BUSY) == 0)
            break;
    }

    // wait for card to be ready for new commands
    for (int i = 0; i < CF_CARD_TIMEOUT; i++)
    {
        if ((*statusReg & CF_STS_INSERTED) != 0)
        {
            return true;
        }
    }

    return false;
}

bool CompactFlashCommonLoaderPlatform::InitializeCFCard()
{
    if(!RequiresLocking())
    {
        const u16 noLockingOpcode = THUMB_MOVS_REG(THUMB_R0, THUMB_R0);
        CF_PerformTransfer_unlock_label = noLockingOpcode;
        CF_PerformTransfer_lock_label = noLockingOpcode;
    }
    const auto& regs = GetCfRegisters();
    if(!waitAvailableForCommands(regs))
    {
        return false;
    }
    auto lba1 = (vu16*)regs.lba1;
    u16 temp = *lba1;
    *lba1 = (~temp & 0xFF);
    temp = (~temp & 0xFF);
    if (*lba1 != temp)
    {
        return false;
    }

    *lba1 = 0xAA55;
    if (*lba1 == 0xAA55)
    {
        return false;
    }
    return true;
}
