#include "common.h"
#include <libtwl/card/card.h>
#include "../SdioDefinitions.h"
#include "thumbInstructions.h"
#include "Ez5hLoaderPlatform.h"

// ROMCTRL flags
#define EZ5H_CTRL_BASE                                                                 \
    (MCCNT1_ENABLE | MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE | MCCNT1_CLOCK_SCRAMBLER | \
     MCCNT1_READ_DATA_DESCRAMBLE | MCCNT1_LATENCY2(24) | MCCNT1_LATENCY1(0))
#define EZ5H_CTRL_READ_4B (EZ5H_CTRL_BASE | MCCNT1_LEN_4)
#define EZ5H_CTRL_READ_512B (EZ5H_CTRL_BASE | MCCNT1_LEN_512)

// CARD_COMMANDs
#define EZ5H_CMD_SDMC (0xB800000000000000ull)
#define EZ5H_CMD_SDMC_READ_DATA (EZ5H_CMD_SDMC | 0x00F7000000000000ull)

static constexpr inline u64 EZ5H_CMD_SDMC_PARAM_CARD(u8 idx, u8 cmd, u32 parameter) {
    return (EZ5H_CMD_SDMC | 0x00FA000000000000ull | ((u64)idx << 40) | ((u64)cmd << 32) |
            (u64)parameter);
}

static constexpr inline u64 EZ5H_CMD_SDMC_SDIO(u8 cmd, u32 parameter) {
    return EZ5H_CMD_SDMC_PARAM_CARD(0, cmd | 0x40, parameter);
}

// Sends a clock, reads data from response index if available
static constexpr inline u64 EZ5H_CMD_SDMC_SEND_CLK(u8 idx) {
    return EZ5H_CMD_SDMC_PARAM_CARD(idx, 0, 0);
}

static void romDrainData() {
    card_romSetCmd(EZ5H_CMD_SDMC_READ_DATA);
    card_romStartXfer(EZ5H_CTRL_READ_512B, false);
    while (card_romIsBusy())
	{
		if (card_romIsDataReady())
		{
			(void)card_romGetData();
		}
	}
}

static u32 EZ5H_SendCommand(const u64 command) {
    card_romSetCmd(command);
    card_romStartXfer(EZ5H_CTRL_READ_4B | MCCNT1_LEN_4, false);
    card_romWaitDataReady();
    return card_romGetData();
}

static bool EZ5H_SDSendSDIOCommand(u8 cmd, u32 parameter, u8* buffer, int size) {
    u32 data;
    u8* u8_data = (u8*)&data;
    int timeout = 99;

    // Either it's no response, 48 bits, or 136 bits.
    // If this isn't the case, then this function doesn't work.
    if (size != 0 && size != 6 && size != 17) return false;

    EZ5H_SendCommand(EZ5H_CMD_SDMC_SDIO(cmd, parameter));

    // R0 has no response.
    if (size == 0) return true;

    // Sends response in byte-swapped u32, with the starting marker
    // Search for starting marker, with a timeout
    do {
        data = EZ5H_SendCommand(EZ5H_CMD_SDMC_SEND_CLK(1));
        timeout--;
        if (!timeout) return false;
    } while (data & 0xFF);

    // Starting marker found. Start reading response
    if (buffer != nullptr) {
        buffer[0] = u8_data[1];
        buffer[1] = u8_data[2];
        buffer[2] = u8_data[3];
    }

    // Read remaining data
    data = EZ5H_SendCommand(EZ5H_CMD_SDMC_SEND_CLK(2));
    if (buffer != nullptr) {
        buffer[3] = u8_data[0];
        buffer[4] = u8_data[1];
        buffer[5] = u8_data[2];
        // if we're pulling an R1 response, then we have read all of our data here
        // otherwise keep going
        if (size != 6) return true;
        buffer[6] = u8_data[3];
    } else if (size == 6)
        return true;

    data = EZ5H_SendCommand(EZ5H_CMD_SDMC_SEND_CLK(3));
    if (buffer != nullptr) {
        buffer[7] = u8_data[0];
        buffer[8] = u8_data[1];
        buffer[9] = u8_data[2];
        buffer[10] = u8_data[3];
    }
    data = EZ5H_SendCommand(EZ5H_CMD_SDMC_SEND_CLK(4));
    if (buffer != nullptr) {
        buffer[11] = u8_data[0];
        buffer[12] = u8_data[1];
        buffer[13] = u8_data[2];
        buffer[14] = u8_data[3];
    }
    data = EZ5H_SendCommand(EZ5H_CMD_SDMC_SEND_CLK(5));
    if (buffer != nullptr) {
        buffer[15] = u8_data[0];
        buffer[16] = u8_data[1];
    }

    return true;
}

bool Ez5hLoaderPlatform::InitializeSdCard() {
    u8 response[17] = {};
    bool isSD20 = false;

    // Does this flush something?
    // iSmart does this loop
    for (int i = 0; i < 128; i++)
        romDrainData();
    EZ5H_SDSendSDIOCommand(SD_CMD0_GO_IDLE_STATE, 0, nullptr, 0);
    // it does it twice
    for (int i = 0; i < 128; i++)
        romDrainData();

    // CMD8 SDHC init
    if (EZ5H_SDSendSDIOCommand(SD_CMD8_SEND_IF_COND, 0x1AA, response, 6))
        if (response[3] == 1 && response[4] == 0xAA) isSD20 = true;

    do {
        EZ5H_SDSendSDIOCommand(SD_CMD55_APP_CMD, 0, nullptr, 6);
        u32 parameter = 0x00800000;
        if (isSD20) parameter |= BIT(30);
        EZ5H_SDSendSDIOCommand(SD_ACMD41_SD_SEND_OP_COND, parameter, response, 6);
    } while (!(response[1] & 0x80));

    bool isSdhc = response[1] & 0x40;

    EZ5H_SDSendSDIOCommand(SD_CMD2_ALL_SEND_CID, 0, nullptr, 17);
    do {
        EZ5H_SDSendSDIOCommand(SD_CMD3_SEND_RELATIVE_ADDR, 0, response, 6);
    } while ((response[3] & 0x1E) != 6);  // is standby

    u32 sdio_rca = (response[1] << 8) + response[2];

    EZ5H_SDSendSDIOCommand(SD_CMD9_SEND_CSD, (sdio_rca << 16), nullptr, 17);
    EZ5H_SDSendSDIOCommand(SD_CMD7_SELECT_CARD, (sdio_rca << 16), nullptr, 6);
    EZ5H_SDSendSDIOCommand(SD_CMD55_APP_CMD, (sdio_rca << 16), nullptr, 6);
    EZ5H_SDSendSDIOCommand(SD_ACMD6_SET_BUS_WIDTH, 2, nullptr, 6);
    EZ5H_SDSendSDIOCommand(SD_CMD16_SET_BLOCKLEN, 512, nullptr, 6);

    const u16 nonSdhcOpcode = THUMB_LSLS_IMM(THUMB_R1, THUMB_R0, 9);
    const u16 sdhcOpcode = THUMB_MOVS_REG(THUMB_R1, THUMB_R0);
    const u16 opcode = isSdhc ? sdhcOpcode : nonSdhcOpcode;
	ez5h_sdhc_read_label = opcode;
	ez5h_sdhc_readDma_label = opcode;
	ez5h_sdhc_write_label = opcode;
    return true;
}
