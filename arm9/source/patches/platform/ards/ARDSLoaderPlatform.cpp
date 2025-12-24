/*
    ARDS - Datel Action Replay DS
    SD intialization routine

    Copyright (C) 2025 Edoardo Lolletti (edo9300)

    SPDX-License-Identifier: Zlib
*/

#include "common.h"
#include "ARDSLoaderPlatform.h"
#include "../SdioDefinitions.h"
#include <libtwl/card/card.h>
#include <libtwl/mem/memExtern.h>
#include "libtwl_ext.h"
#include "ards.h"
#include "thumbInstructions.h"

static void ARDSLoader_SendNtrCommandF2(uint32_t param1, uint8_t param2) {
    cardExt_SendCommand(ARDS_CMD_F2(param1, param2), ARDS_CTRL_BASE);
}

static uint8_t ARDSLoader_ReadSpiByte(void) {
    return cardExt_ReadWriteSpiByte(ARDS_SPI_READ_BYTE);
}

static void ARDSLoader_CycleSpi() {
    ARDSLoader_SendNtrCommandF2(0, ARDS_CMD_F2_SPI_DISABLE);
    cardExt_EnableSpi();
	ARDSLoader_ReadSpiByte();
    ARDSLoader_SendNtrCommandF2(0, ARDS_CMD_F2_SPI_ENABLE);
    cardExt_EnableSpi();
}

// Sends SDIO command to ARDS.
static uint8_t ARDSLoader_SpiSendSDIOCommand(uint8_t cmdId, uint32_t arg, uint8_t * buffer, int len)
{
	ARDSLoader_CycleSpi();
    uint8_t cmd[6];

    // Build a SPI SD command to be sent as-is.
    cmd[0] = 0x40 | (cmdId & 0x3f);
    cmd[1] = arg >> 24;
    cmd[2] = arg >> 16;
    cmd[3] = arg >> 8;
    cmd[4] = arg >> 0;
    // CRC in SPI mode is ignored for every command but CMD0 (hardcoded to 0x95)
    // and CMD8, hardcoded to 0x86 with the default 0x1AA argument.
    cmd[5] = (len > 0) ? 0x86 : 0x95;

    for (int i = 0; i < sizeof(cmd); i++) cardExt_ReadWriteSpiByte(cmd[i]);

    uint8_t timeout = ARDS_ReadSpiByteTimeout();

    const uint8_t * target = buffer == NULL ? NULL : (buffer + len);
    for(int i=0; i < len; i++)
    {
        uint8_t data = ARDSLoader_ReadSpiByte();
        if(buffer < target)
            *buffer++ = data;
    }

    return timeout;
}

static uint8_t ARDSLoader_SpiSendSDIOCommandR0(uint8_t cmd, uint32_t arg)
{
    return ARDSLoader_SpiSendSDIOCommand(cmd, arg, NULL, 0);
}

bool ARDSLoaderPlatform::InitializeSdCard()
{
    bool isv2 = false;
	bool isSdhc = false;
    for (int i = 0; i < 0x100; i++) {
        ARDSLoader_SendNtrCommandF2(0x7FFFFFFF | ((i & 1) << 31), 0x00);
    }

    // Send CMD0.
    uint8_t r1 = ARDSLoader_SpiSendSDIOCommandR0(0, 0);
    if (r1 != 0x01)  // Idle State.
    {
        // CMD 0 failed.
        return false;
    }

    uint32_t r7_answer;

    r1 = ARDSLoader_SpiSendSDIOCommand(8, 0x1AA, (uint8_t*)&r7_answer, 4);

    uint32_t acmd41_arg = 0;

    if (r1 == 0x1 && r7_answer == 0xAA010000) {
        isv2 = true;
        acmd41_arg |= (1 << 30);  // Set HCS bit,Supports SDHC
    }

    for (int i = 0; i < ARDS_MAX_STARTUP_TRIES; ++i) {
        // Send ACMD41.
        ARDSLoader_SpiSendSDIOCommandR0(ARDS_SDIO_CMD55_APP_CMD, 0);
        r1 = ARDSLoader_SpiSendSDIOCommandR0(ARDS_SDIO_ACMD41_SD_SEND_OP_COND, acmd41_arg);
        if (r1 == 0) {
            break;
        }
    }
    if (r1 != 0) return false;

    if (isv2) {
        uint32_t r2_answer;
        r1 = ARDSLoader_SpiSendSDIOCommand(ARDS_SDIO_CMD58_READ_OCR, 0, (uint8_t*)&r2_answer, 4);
        isSdhc = (r2_answer & 0x40) != 0;
    }
    ARDSLoader_SpiSendSDIOCommandR0(ARDS_SDIO_CMD16_SET_BLOCK_LEN, 0x200);

    const u16 nonSdhcOpcode = THUMB_LSLS_IMM(THUMB_R1, THUMB_R0, 9);
    const u16 sdhcOpcode = THUMB_MOVS_REG(THUMB_R1, THUMB_R0);
    const u16 opcode = isSdhc ? sdhcOpcode : nonSdhcOpcode;
    ARDS_writeSectorSdhcLabel = opcode;
    ARDS_readSectorSdhcLabel = opcode;

    return true;
}
