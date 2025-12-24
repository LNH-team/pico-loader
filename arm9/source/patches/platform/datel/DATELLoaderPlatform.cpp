#include "common.h"
#include <libtwl/card/card.h>
#include "DATELLoaderPlatform.h"
#include "../SdioDefinitions.h"
#include "thumbInstructions.h"

static constexpr size_t MAX_STARTUP_TRIES = 5000;
static constexpr uint32_t DATEL_CTRL_BASE = (MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_LATENCY2(0x3F));

static constexpr uint8_t DATEL_CMD_F2_SPI_ENABLE = 0xCC;
static constexpr uint8_t DATEL_CMD_F2_SPI_DISABLE = 0xC8;

static inline u64 DATEL_CMD_F2(u32 param1, u8 param2) {
    return (0xF200000000000000ull | ((u64)param1 << 24) | ((u64)param2 << 16));
}

static inline void EnableSpi()
{
    REG_MCCNT0 = (REG_MCCNT0 & ~(MCCNT0_MODE_MASK | MCCNT0_ROM_XFER_IRQ)) | MCCNT0_MODE_SPI | MCCNT0_SPI_HOLD_CS | MCCNT0_ENABLE;
}

static uint8_t ReadWriteSpiByte(uint8_t data)
{
    REG_MCD0 = data;
    while(REG_MCCNT0 & MCCNT0_SPI_BUSY);
    return REG_MCD0;
}

static void SendNtrCommandF2(uint32_t param1, uint8_t param2) {
    card_romSetCmd(DATEL_CMD_F2(param1, param2));
    card_romStartXfer(DATEL_CTRL_BASE | MCCNT1_LEN_0, false);
    card_romWaitBusy();
}

static void CycleSpi() {
    SendNtrCommandF2(0, DATEL_CMD_F2_SPI_DISABLE);
    EnableSpi();
    ReadWriteSpiByte(0xFF);
    SendNtrCommandF2(0, DATEL_CMD_F2_SPI_ENABLE);
    EnableSpi();
}

// Sends SDIO command to DATEL device.
static uint8_t SpiSendSDIOCommand(uint8_t cmdId, uint32_t arg, uint8_t * buffer, int messageLen)
{
    CycleSpi();
    uint8_t cmd[6];

    // Build a SPI SD command to be sent as-is.
    cmd[0] = 0x40 | (cmdId & 0x3f);
    cmd[1] = arg >> 24;
    cmd[2] = arg >> 16;
    cmd[3] = arg >> 8;
    cmd[4] = arg >> 0;
    // CRC in SPI mode is ignored for every command but CMD0 (hardcoded to 0x95)
    // and CMD8, hardcoded to 0x86 with the default 0x1AA argument.
    cmd[5] = (messageLen > 1) ? 0x86 : 0x95;

    for (auto byte : cmd) ReadWriteSpiByte(byte);

    uint8_t timeout = DATEL_ReadSpiByteTimeout();

    for(int i = 0; i < (messageLen - 1); i++)
    {
        buffer[i] = ReadWriteSpiByte(0xFF);
    }

    return timeout;
}

static uint8_t SpiSendSDIOCommandR0(uint8_t cmd, uint32_t arg)
{
    return SpiSendSDIOCommand(cmd, arg, nullptr, 1);
}

bool DATELLoaderPlatform::InitializeSdCard()
{
    for (int i = 0; i < 0x100; i++)
    {
        SendNtrCommandF2(0x7FFFFFFF | ((i & 1) << 31), 0x00);
    }

    // Send CMD0.
    if (SpiSendSDIOCommandR0(SD_CMD0_GO_IDLE_STATE, 0) != 0x01)
    {
        return false;
    }

    uint32_t cmd8_answer{};

    uint32_t acmd41_arg = 0;
    bool isv2 = false;

    if (SpiSendSDIOCommand(SD_CMD8_SEND_IF_COND, SD_IF_COND_PATTERN, (uint8_t*)&cmd8_answer, 5) == 0x1
        && cmd8_answer == 0xAA010000)
    {
        isv2 = true;
        acmd41_arg |= (1 << 30);  // Set HCS bit,Supports SDHC
    }

    {
        size_t i;
        for (i = 0; i < MAX_STARTUP_TRIES; ++i)
        {
            // Send ACMD41.
            SpiSendSDIOCommandR0(SD_CMD55_APP_CMD, 0);
            if (SpiSendSDIOCommandR0(SD_ACMD41_SD_SEND_OP_COND, acmd41_arg) == 0)
            {
                break;
            }
        }
        if (i >= MAX_STARTUP_TRIES)
        {
            return false;
        }
    }

    bool isSdhc = false;
    if (isv2)
    {
        uint32_t cmd58_answer{};
        SpiSendSDIOCommand(SD_SPI_CMD58_READ_OCR, 0, (uint8_t*)&cmd58_answer, 5);
        isSdhc = (cmd58_answer & 0x40) != 0;
    }
    SpiSendSDIOCommandR0(SD_CMD16_SET_BLOCKLEN, 0x200);

    const u16 nonSdhcOpcode = THUMB_LSLS_IMM(THUMB_R0, THUMB_R0, 9);
    const u16 sdhcOpcode = THUMB_MOVS_REG(THUMB_R0, THUMB_R0);
    const u16 opcode = isSdhc ? sdhcOpcode : nonSdhcOpcode;
    DATEL_writeSectorSdhcLabel = opcode;
    DATEL_readSectorSdhcLabel = opcode;

    return true;
}
