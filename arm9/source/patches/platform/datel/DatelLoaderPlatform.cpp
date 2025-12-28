#include "common.h"
#include <libtwl/card/card.h>
#include "../SdioDefinitions.h"
#include "thumbInstructions.h"
#include "DatelLoaderPlatform.h"

static constexpr int MAX_STARTUP_TRIES = 5000;
static constexpr int SD_COMMAND_TIMEOUT = 0xFFF;
static constexpr u32 DATEL_CTRL_BASE = (MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_LATENCY2(0x3F));

static constexpr u8 DATEL_CMD_F2_SPI_ENABLE = 0xCC;
static constexpr u8 DATEL_CMD_F2_SPI_DISABLE = 0xC8;

static inline u64 DATEL_CMD_F2(u32 param1, u8 param2)
{
    return (0xF200000000000000ull | ((u64)param1 << 24) | ((u64)param2 << 16));
}

static inline void enableSpi()
{
    REG_MCCNT0 = MCCNT0_MODE_SPI | MCCNT0_SPI_HOLD_CS | MCCNT0_ENABLE;
}

static u8 readWriteSpiByte(u8 data)
{
    REG_MCD0 = data;
    while(REG_MCCNT0 & MCCNT0_SPI_BUSY);
    return REG_MCD0;
}

static u8 readSpiByteTimeout()
{
    auto timeout = SD_COMMAND_TIMEOUT;
    u8 res;
    do
    {
        res = readWriteSpiByte(0xFF);
    } while (res == 0xFF && --timeout > 0);
    return res;
}

static void sendNtrCommandF2(u32 param1, u8 param2)
{
    card_romSetCmd(DATEL_CMD_F2(param1, param2));
    card_romStartXfer(DATEL_CTRL_BASE | MCCNT1_LEN_0, false);
    card_romWaitBusy();
}

static void cycleSpi()
{
    sendNtrCommandF2(0, DATEL_CMD_F2_SPI_DISABLE);
    enableSpi();
    readWriteSpiByte(0xFF);
    sendNtrCommandF2(0, DATEL_CMD_F2_SPI_ENABLE);
    enableSpi();
}

// Sends SDIO command to DATEL device.
static u8 spiSendSdioCommand(u8 cmdId, u32 arg, u8 * buffer, int messageLen)
{
    cycleSpi();
    u8 cmd[6];

    // Build a SPI SD command to be sent as-is.
    cmd[0] = 0x40 | (cmdId & 0x3f);
    cmd[1] = arg >> 24;
    cmd[2] = arg >> 16;
    cmd[3] = arg >> 8;
    cmd[4] = arg >> 0;
    // CRC in SPI mode is ignored for every command but CMD0 (hardcoded to 0x95)
    // and CMD8, hardcoded to 0x86 with the default 0x1AA argument.
    cmd[5] = (messageLen > 1) ? 0x86 : 0x95;

    for (u8 byte : cmd)
    {
        readWriteSpiByte(byte);
    }

    u8 timeout = readSpiByteTimeout();

    for (int i = 0; i < (messageLen - 1); i++)
    {
        buffer[i] = readWriteSpiByte(0xFF);
    }

    return timeout;
}

static u8 spiSendSdioCommandR0(u8 cmd, u32 arg)
{
    return spiSendSdioCommand(cmd, arg, nullptr, 1);
}

static bool trySendAcmd41(u32 acmd41Arg)
{
    for (int i = 0; i < MAX_STARTUP_TRIES; i++)
    {
        // Send ACMD41.
        spiSendSdioCommandR0(SD_CMD55_APP_CMD, 0);
        if (spiSendSdioCommandR0(SD_ACMD41_SD_SEND_OP_COND, acmd41Arg) == 0)
        {
            return true;
        }
    }
    return false;
}

bool DatelLoaderPlatform::InitializeSdCard()
{
    for (int i = 0; i < 0x100; i++)
    {
        sendNtrCommandF2(0x7FFFFFFF | ((i & 1) << 31), 0x00);
    }

    // Send CMD0.
    if (spiSendSdioCommandR0(SD_CMD0_GO_IDLE_STATE, 0) != 0x01)
    {
        return false;
    }

    u32 cmd8Answer = 0;

    u32 acmd41Arg = 0;
    bool isV2 = false;

    if (spiSendSdioCommand(SD_CMD8_SEND_IF_COND, SD_IF_COND_PATTERN, (u8*)&cmd8Answer, 5) == 0x1
        && cmd8Answer == 0xAA010000)
    {
        isV2 = true;
        acmd41Arg |= (1 << 30);  // Set HCS bit,Supports SDHC
    }

    if (!trySendAcmd41(acmd41Arg))
    {
        return false;
    }

    bool isSdhc = false;
    if (isV2)
    {
        u32 cmd58Answer = 0;
        spiSendSdioCommand(SD_SPI_CMD58_READ_OCR, 0, (u8*)&cmd58Answer, 5);
        isSdhc = (cmd58Answer & 0x40) != 0;
    }
    spiSendSdioCommandR0(SD_CMD16_SET_BLOCKLEN, 0x200);

    const u16 nonSdhcOpcode = THUMB_LSLS_IMM(THUMB_R0, THUMB_R0, 9);
    const u16 sdhcOpcode = THUMB_MOVS_REG(THUMB_R0, THUMB_R0);
    const u16 opcode = isSdhc ? sdhcOpcode : nonSdhcOpcode;
    datel_writeSectorSdhcLabel = opcode;
    datel_readSectorSdhcLabel = opcode;

    return true;
}
