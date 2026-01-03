#include "common.h"
#include <libtwl/card/card.h>
#include "DsttDefinitions.h"
#include "../SdioDefinitions.h"
#include "waitByLoop.h"
#include "DsttLoaderPlatform.h"

/// @brief Sets the mode of the SD host. Mostly used for SDIO commands.
/// @param sdio An SDIO command to send, if any. Set 0 otherwise.
/// @param parameter Parameter to send alongside the SDIO command, if any. Set 0 otherwise.
/// @param response_type A response type defined in DSTTSdHostModes.
/// @param latency Additional MCCNT1 latency to apply when applying the command.
static u32 sdHostSetMode(u8 sdio, u32 parameter, u8 response_type, u32 latency)
{
    u64 command = ((u64)DSTT_CMD_SD_HOST_PARAM << 56) | ((u64)parameter << 24) | ((u64)sdio << 16) | ((u64)response_type << 8);
    card_romSetCmd(command);
    card_romStartXfer(DSTT_CTRL_READ_4B | MCCNT1_LATENCY1(latency), false);
    card_romWaitDataReady();
    return card_romGetData();
}

/// @brief Checks if the SD host is busy.
/// @return 0 if not busy, 1 otherwise.
static u32 isSdHostBusy(void)
{
    REG_MCCMD0_U8[0] = DSTT_CMD_SD_HOST_BUSY;
    card_romStartXfer(DSTT_CTRL_READ_4B, false);
    card_romWaitDataReady();
    return card_romGetData();
}

/// @brief Retrieves the response from the SD host.
/// @return A u32 response. If the SD host was in SDIO mode, only bits 8-40 are returned.
static u32 sdHostGetResponse(void)
{
    REG_MCCMD0_U8[0] = DSTT_CMD_SD_HOST_RESPONSE;
    card_romStartXfer(DSTT_CTRL_READ_4B, false);
    card_romWaitDataReady();
    return __builtin_bswap32(card_romGetData());
}

/// @brief Sends an SDIO R0-type command to the cartridge.
static void sdSendR0Command(u8 sdio, u32 parameter, u32 latency)
{
    sdHostSetMode(sdio, parameter, DSTT_SD_HOST_NORESPONSE, latency);
    while (isSdHostBusy())
        ;
}

/// @brief Sends an SDIO R1-type command to the cartridge.
/// @return Bits 8-40 of the R1 response.
static u32 sdSendR1Command(u8 sdio, u32 parameter, u32 latency)
{
    sdHostSetMode(sdio, parameter, DSTT_SD_HOST_READ_4B, latency);
    while (isSdHostBusy())
        ;
    return sdHostGetResponse();
}

/// @brief Sends an SDIO R2-type command to the cartridge.
/// TODO: save the response to a buffer (also figure out in which order they're sent)
static void sdSendR2Command(u8 sdio, u32 parameter, u32 latency)
{
    sdHostSetMode(sdio, parameter, DSTT_SD_HOST_READ_4B_MULTI, latency);
    while (isSdHostBusy())
        ;

    // TODO: parse this response
    sdHostGetResponse();

    for (int i = 0; i < 4; i++)
    {
        sdHostSetMode(sdio, parameter, DSTT_SD_HOST_NEXT_4B, latency);
        while (isSdHostBusy())
            ;
        // TODO: parse this response
        sdHostGetResponse();
    }
    sdHostSetMode(sdio, parameter, DSTT_SD_HOST_SEND_STOP_CLK, 0);
    while (isSdHostBusy())
        ;
}

/// @brief Manipulates the cartridge's SD host register.
static void sdHostSetRegister(u8 bits)
{
    u64 command = ((u64)DSTT_CMD_SD_HOST_SET_REGISTER << 56) | ((u64)(0x30 | bits) << 48);
    card_romSetCmd(command);
    card_romStartXfer(DSTT_CTRL_READ_4B, false);
    card_romWaitDataReady();
    card_romGetData();
    waitByLoop(0x600);
}

bool DsttLoaderPlatform::InitializeSdCard(void)
{
    bool isSdVersion2 = false;
    bool isSdhc = false;
    u32 response = 0;

    // TODO: What is this command doing?
    card_romSetCmd(0x6600000000000000ull);
    card_romStartXfer(DSTT_CTRL_SET_CARD_MODE, false);
    card_romWaitDataReady();
    card_romGetData();

    // Reset SD host
    sdHostSetRegister(DSTT_SD_HOST_REG_CLEAR_ALL);

    // Init
    sdHostSetRegister(DSTT_SD_HOST_REG_RESET | DSTT_SD_HOST_REG_400KHZ_CLK | DSTT_SD_HOST_REG_CLEAN_ROM_MODE);
    sdHostSetMode(0, 0, DSTT_SD_HOST_SEND_CLK, DSTT_CTRL_SD_LOW_CLK_LATENCY);
    waitByLoop(0x2000);

    // CMD0
    sdSendR0Command(0, 0, DSTT_CTRL_SD_LOW_CLK_LATENCY);
    sdHostSetMode(SD_CMD0_GO_IDLE_STATE, 0, DSTT_SD_HOST_SEND_STOP_CLK, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    // CMD8
    sdHostSetMode(SD_CMD8_SEND_IF_COND, SD_IF_COND_PATTERN, DSTT_SD_HOST_READ_4B, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    u32 retryCount = 9999;
    while (true)
    {
        if (!isSdHostBusy())
        {
            response = sdHostGetResponse();
            break;
        }
        if (--retryCount == 0)
        {
            sdHostSetRegister(DSTT_SD_HOST_REG_CLEAR_ALL);
            sdHostSetRegister(DSTT_SD_HOST_REG_RESET | DSTT_SD_HOST_REG_400KHZ_CLK | DSTT_SD_HOST_REG_CLEAN_ROM_MODE);
            response = 0;
            break;
        }
    }

    if (response == SD_IF_COND_PATTERN)
    {
        isSdVersion2 = true;
    }

    do
    {
        // CMD55
        sdHostSetMode(SD_CMD55_APP_CMD, 0, DSTT_SD_HOST_READ_4B, DSTT_CTRL_SD_LOW_CLK_LATENCY);
        retryCount = 9999;
        while (isSdHostBusy())
        {
            if (--retryCount == 0)
            {
                sdHostSetRegister(DSTT_SD_HOST_REG_CLEAR_ALL);
                sdHostSetRegister(DSTT_SD_HOST_REG_RESET | DSTT_SD_HOST_REG_400KHZ_CLK | DSTT_SD_HOST_REG_CLEAN_ROM_MODE);
                return false;
            }
        }
        sdHostGetResponse();

        // ACMD41
        u32 parameter = 0x00FC0000;
        if (isSdVersion2)
        {
            parameter |= BIT(30);
        }
        response = sdSendR1Command(SD_ACMD41_SD_SEND_OP_COND, parameter, DSTT_CTRL_SD_LOW_CLK_LATENCY);
    } while (!(response & BIT(31)));

    isSdhc = (response & BIT(30)) != 0;

    // CMD2
    sdSendR2Command(SD_CMD2_ALL_SEND_CID, 0, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    // CMD3
    response = sdSendR1Command(SD_CMD3_SEND_RELATIVE_ADDR, 0, DSTT_CTRL_SD_LOW_CLK_LATENCY);
    u32 sdioRca = response & 0xFFFF0000;

    // CMD7
    sdSendR1Command(SD_CMD7_SELECT_CARD, sdioRca, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    // ACMD6
    sdSendR1Command(SD_CMD55_APP_CMD, sdioRca, DSTT_CTRL_SD_LOW_CLK_LATENCY);
    sdSendR1Command(SD_ACMD6_SET_BUS_WIDTH, 2, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    // CMD16
    sdSendR1Command(SD_CMD16_SET_BLOCKLEN, 512, DSTT_CTRL_SD_LOW_CLK_LATENCY);

    sdHostSetRegister(DSTT_SD_HOST_REG_CLEAR_ALL);
    sdHostSetRegister(DSTT_SD_HOST_REG_RESET | DSTT_SD_HOST_REG_CLEAN_ROM_MODE);
    if (isSdhc)
    {
        sdHostSetRegister(DSTT_SD_HOST_REG_RESET | DSTT_SD_HOST_REG_CLEAN_ROM_MODE | DSTT_SD_HOST_REG_SDHC);
        dstt_readSd_sdsc_shift = THUMB_MOVS_REG(THUMB_R7, THUMB_R0);
        dstt_writeSd_sdsc_shift = THUMB_NOP;
    }
    return true;
}
