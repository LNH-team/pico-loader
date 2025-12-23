/*
    ARDS - Datel Action Replay DS
    Card IO routines

    Copyright (C) 2023 lifehackerhansol
    Copyright (C) 2025 Edoardo Lolletti (edo9300)

    SPDX-License-Identifier: Zlib
*/

#pragma once

#include <nds/ndstypes.h>
#include <libtwl/card/card.h>

#ifndef NULL
#define NULL 0
#endif

#define ARDS_SDIO_CMD0_GO_IDLE_STATE 0
#define ARDS_SDIO_CMD8_SEND_IF_COND 8
#define ARDS_SDIO_CMD12_STOP_TRANSMISSION 12
#define ARDS_SDIO_CMD16_SET_BLOCK_LEN 16
#define ARDS_SDIO_CMD17_READ_SINGLE_BLOCK 17
#define ARDS_SDIO_CMD18_READ_MULTIPLE_BLOCK 18
#define ARDS_SDIO_CMD24_WRITE_SINGLE_BLOCK 24
#define ARDS_SDIO_CMD25_WRITE_MULTIPLE_BLOCK 25
#define ARDS_SDIO_CMD55_APP_CMD 55
#define ARDS_SDIO_CMD58_READ_OCR 58

#define ARDS_SDIO_ACMD41_SD_SEND_OP_COND 41

#define ARDS_SD_WRITE_OK 0x5

#define ARDS_SD_CMD_TIMEOUT_LEN 0xFFF
#define ARDS_SD_WRITE_TIMEOUT_LEN 0xFFFF

#define ARDS_SPI_READ_BYTE 0xFF
#define ARDS_SPI_START_DATA_TOKEN 0xFE
#define ARDS_SPI_MULTI_BLOCK_WRITE_TOKEN 0xFC
#define ARDS_SPI_END_MULTI_BLOCK_WRITE 0xFD

#define ARDS_CTRL_BASE (MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_LATENCY2(0x3F))

#define ARDS_CMD_F2_SPI_ENABLE 0xCC
#define ARDS_CMD_F2_SPI_DISABLE 0xC8

#define ARDS_MAX_STARTUP_TRIES 5000

static inline u64 ARDS_CMD_F2(u32 param1, u8 param2) {
    return (0xF200000000000000ull | ((u64)param1 << 24) | ((u64)param2 << 16));
}

// User API

bool ARDS_SDInitialize(void);
bool ARDS_SDReadSingleSector(u32 sector, u8 * buffer);
bool ARDS_SDReadMultipleSector(u32 sector, u8 * buffer, u32 num_sectors);
bool ARDS_SDWriteSingleSector(u32 sector, const u8 * buffer);
bool ARDS_SDWriteMultipleSector(u32 sector, const u8 * buffer, u32 num_sectors);
