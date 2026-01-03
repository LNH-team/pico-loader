#pragma once
#include <libtwl/card/card.h>

/// All RPG-based SDIO drivers use this as base, and add latency/size as needed
#define IORPG_CTRL_BASE      (MCCNT1_CMD_SCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_LATENCY2(0) | MCCNT1_LATENCY1(0))
#define IORPG_CTRL_POLL      (IORPG_CTRL_BASE | MCCNT1_ENABLE | MCCNT1_RESET_OFF | MCCNT1_LEN_0)
#define IORPG_CTRL_READ_512B (IORPG_CTRL_POLL | MCCNT1_LEN_512)

/// This command instructs the SD host to switch to SDHC mode.
#define IORPG_CMD_SET_SD_MODE_SDHC (0xC101000000000000ull)

/// @brief SDIO parameter types. Used with IORPG_CMD_SDIO command.
enum IoRpgSdioParamTypes
{
    IORPG_SDIO_NORESPONSE = 0ull,
    IORPG_SDIO_READ_RESPONSE = 1ull,
    IORPG_SDIO_READ_SINGLE_BLOCK = 3ull,
    IORPG_SDIO_READ_MULTI_BLOCK = 4ull,
    IORPG_SDIO_WRITE_SINGLE_BLOCK = 5ull,
    IORPG_SDIO_WRITE_MULTI_BLOCK = 6ull
};
