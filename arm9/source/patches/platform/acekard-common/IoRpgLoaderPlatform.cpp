#include "common.h"
#include <libtwl/card/card.h>
#include "IoRpgDefinitions.h"
#include "../SdioDefinitions.h"
#include "waitByLoop.h"
#include "IoRpgLoaderPlatform.h"

bool IoRpgLoaderPlatform::InitializeSdCard()
{
    bool isSdhc = false;
    bool isSdVersion2 = false;
    u32 responseR1 = 0;

    // CMD0
    SdSendSdioCommand(IoRpgCmdSdio(SD_CMD0_GO_IDLE_STATE, IORPG_SDIO_NORESPONSE, 0), nullptr, 0);

    // CMD8
    responseR1 = SdSendR1Command(SD_CMD8_SEND_IF_COND, SD_IF_COND_PATTERN);
    isSdVersion2 = responseR1 == SD_IF_COND_PATTERN;

    for (u32 i = 0; i < 10000; i++)
    {
        // CMD55
        SdSendR1Command(SD_CMD55_APP_CMD, 0);

        // ACMD41
        u32 argument = 0x00FF8000;
        if (isSdVersion2)
        {
            argument |= BIT(30);
        }
        responseR1 = SdSendR1Command(SD_ACMD41_SD_SEND_OP_COND, argument);
        if (responseR1 & BIT(31))
        {
            isSdhc = responseR1 & BIT(30);
            break;
        }
        waitByLoop(1666);
    }

    // CMD2
    SdSendR2Command(SD_CMD2_ALL_SEND_CID, 0);

    // CMD3
    responseR1 = SdSendR1Command(SD_CMD3_SEND_RELATIVE_ADDR, 0);
    u32 sdioRca = responseR1 >> 16;

    // CMD9
    SdSendR2Command(SD_CMD9_SEND_CSD, (sdioRca << 16));

    // CMD7
    SdSendR1Command(SD_CMD7_SELECT_CARD, (sdioRca << 16));

    // ACMD6
    SdSendR1Command(SD_CMD55_APP_CMD, (sdioRca << 16));
    SdSendR1Command(SD_ACMD6_SET_BUS_WIDTH, 2);

    // CMD13
    responseR1 = SdSendR1Command(SD_CMD13_SEND_STATUS, (sdioRca << 16));
    bool isSuccess = (responseR1 >> 9) == 4;

    if (isSuccess && isSdhc)
    {
        card_romSetCmd(IORPG_CMD_SET_SD_MODE_SDHC);
        card_romStartXfer(IORPG_CTRL_POLL, false);
        card_romWaitBusy();
        PatchSdscShift();
    }

    return isSuccess;
}

void IoRpgLoaderPlatform::SdSendSdioCommand(u64 command, u8* buffer, u32 length) const
{
    u32 flags;
    if (length == 0)
    {
        flags = IORPG_CTRL_POLL | MCCNT1_LATENCY1(80);
    }
    else
    {
        // Actually the original driver used 4KB reads.
        // But with SDIO, it only really reads up to 136 bytes,
        // so 4KB is unnecessary.
        flags = IORPG_CTRL_READ_512B | MCCNT1_LATENCY1(40);
    }

    // param SDIO, get response
    card_romSetCmd(command);
    card_romStartXfer(flags, false);

    // don't poll if we aren't reading
    if (length == 0)
    {
        card_romWaitBusy();
        return;
    }

    u32 i = 0;
    u32 data[2];
    u8* target = buffer + (length >> 3);
    do
    {
        // Read data if available
        if (card_romIsDataReady())
        {
            data[i & 1] = card_romGetData();
            if (i & 1)
            {
                target--;
                // Since every byte is a bit, read two words,
                // then pack it into a single byte to represent the
                // true value
                // BIT(7) is the needed bit from the return values
                for (int j = 0; j < 8; ++j)
                {
                    u8 bit = (((u8*)data)[7 - j] & BIT(7)) >> 7;
                    *target |= bit << j;
                }
            }
            i++;
        }
        // Exit loop when we reach the number of words needed
        if (i >= (length >> 2))
        {
            break;
        }
    } while (card_romIsBusy());

    // Here we cut the transfer short. We don't actually read all 512 bytes as seen
    // in MCCNT1.
    // Cutting the transfer length in the middle of transfer causes undefined
    // behaviour, so wait until then
    card_romWaitDataReady();
    // add a delay just in case
    waitByLoop(33);
    // Cut transmission
    REG_MCCNT1 = 0;
    card_romStartXfer(IORPG_CTRL_POLL, false);
    card_romWaitBusy();
}

u32 IoRpgLoaderPlatform::SdSendR1Command(u8 cmd, u32 argument) const
{
    u64 buffer = 0;
    SdSendSdioCommand(IoRpgCmdSdio(cmd, IORPG_SDIO_READ_RESPONSE, argument), (u8*)&buffer, SD_R1_RESPONSE_LENGTH_BITS);

    return (u32)(buffer >> 9);
}

void IoRpgLoaderPlatform::SdSendR2Command(u8 cmd, u32 argument) const
{
    ALIGN(4) u8 ret[136 >> 3] = {};
    SdSendSdioCommand(IoRpgCmdSdio(cmd, IORPG_SDIO_READ_RESPONSE, argument), ret, SD_R2_RESPONSE_LENGTH_BITS);
}
