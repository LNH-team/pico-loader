#pragma once

#include "common.h"
#include <libtwl/card/card.h>

/// libtwl workaround. Certain DSTT clones need bytewise access to MCCMD
#define REG_MCCMD0_U8   ((vu8*)&REG_MCCMD0)

/// Common DSTT MCCNT1 flags.
#define DSTT_CTRL_BASE          (MCCNT1_ENABLE | MCCNT1_RESET_OFF | MCCNT1_LATENCY2(24) | MCCNT1_LATENCY1(0))
#define DSTT_CTRL_READ_4B       (DSTT_CTRL_BASE | MCCNT1_LEN_4)
#define DSTT_CTRL_SET_CARD_MODE (DSTT_CTRL_READ_4B | MCCNT1_CMD_SCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | MCCNT1_READ_DATA_DESCRAMBLE)

/// Common MCCNT1 LATENCY1.
/// During SDIO init SD host commands have a higher latency; in all other cases this is 0.
#define DSTT_CTRL_SD_LOW_CLK_LATENCY 0x1000

/// SD host related commands.
///
/// Note:
///     While this is where the SDIO happens, it isn't always SDIO.
///     Thus, it can sometimes be 0.
///
/// Command structure:
/// 51 AA AA AA AA BB CC 00
/// AAAAAAAA = SDIO parameter
/// BB = SDIO command
/// CC = SD host mode, see DSTTSdHostModes enum.
#define DSTT_CMD_SD_HOST_PARAM        0x51

/// SD host miscellaneous commands.
/// return 0 == idle
/// return non-0 == not idle
#define DSTT_CMD_SD_HOST_BUSY         0x50

/// Retrieves the response from previous SD host param.
/// Returns a value if the sent mode is 1 or 2.
#define DSTT_CMD_SD_HOST_RESPONSE     0x52

/// Command to write to host register. The register bits can be found below.
#define DSTT_CMD_SD_HOST_SET_REGISTER 0x5F

/// SD host modes.
/// Used with DSTT_CMD_SD_HOST_PARAM command.
enum DSTTSdHostModes
{
    DSTT_SD_HOST_NORESPONSE = 0,
    DSTT_SD_HOST_READ_4B = 1,
    DSTT_SD_HOST_READ_4B_MULTI = 2,      // use mode 3 to continue this read
    DSTT_SD_HOST_NEXT_4B = 3,
    DSTT_SD_HOST_SEND_CLK = 4,
    DSTT_SD_HOST_SEND_STOP_CLK = 5,
    DSTT_SD_HOST_READ_DATABLOCK = 6,
    DSTT_SD_HOST_NEXT_DATABLOCK = 7,
    DSTT_SD_HOST_CMD17_READ_DATA = 8,    // Send SDIO CMD17 & read data
    DSTT_SD_HOST_CMD18_READ_DATA = 9,    // Send SDIO CMD18 & read data until stop
    DSTT_SD_HOST_COMMIT_FIFO_DATA = 0xA, // commit data in FIFO to SD card
    DSTT_SD_HOST_CMD24_WRITE_DATA = 0xB, // Send SDIO CMD24 & send data in SRAM buffer
    DSTT_SD_HOST_WAIT_BUSY = 0xC         // wait until data transfer ends
};

/// SD host control registers.
/// The 0x5F sets raw registers related to the SD host, which is a single u8.
/// Bits:
///     0: Reset
///     1: Set 400k low clk
///     2: Use 0xB7 as alternative of 0x5B for ROM reads
///     3: Set SDHC mode
///     4-5: 1
///     6-7: 0
#define DSTT_SD_HOST_REG_CLEAR_ALL      0
#define DSTT_SD_HOST_REG_RESET          BIT(0)
#define DSTT_SD_HOST_REG_400KHZ_CLK     BIT(1)
#define DSTT_SD_HOST_REG_CLEAN_ROM_MODE BIT(2)
#define DSTT_SD_HOST_REG_SDHC           BIT(3)
