#include "common.h"
#include <libtwl/mem/memExtern.h>
#include "../SdioDefinitions.h"
#include "thumbInstructions.h"
#include "SuperCardDefinitions.h"
#include "SuperCardLoaderPlatform.h"
#include "SuperCardSdCommands.h"

enum SupercardType
{
    SUPERCARD_TYPE_SC_SD = 0x00,
    SUPERCARD_TYPE_SC_LITE = 0x01,
    SUPERCARD_TYPE_SC_CF = 0x02,
    SUPERCARD_TYPE_CHIS = 0x04,
    SUPERCARD_TYPE_SC_RUMBLE = (0x10 | SUPERCARD_TYPE_SC_LITE),
    SUPERCARD_TYPE_UNK = ~SUPERCARD_TYPE_SC_RUMBLE,
};

static void changeSupercardMode(u16 mode)
{
    SC_MODE_REG = SC_MODE_MAGIC;
    SC_MODE_REG = SC_MODE_MAGIC;
    SC_MODE_REG = mode;
    SC_MODE_REG = mode;
}

static SupercardType detectSupercardType()
{
    changeSupercardMode(SC_MODE_SDCARD);
    u32 val = *(vu16*)0x09800000;
    switch (val & 0xE300)
    {
        case 0xA000:
        {
            return SUPERCARD_TYPE_SC_LITE;
        }
        case 0xC000:
        {
            return SUPERCARD_TYPE_SC_RUMBLE;
        }
        case 0xE000:
        {
            return SUPERCARD_TYPE_SC_SD;
        }
        default:
        {
            return SUPERCARD_TYPE_CHIS;
        }
    }
}

bool SuperCardLoaderPlatform::InitializeSdCard()
{
    u32 oldMemCnt = REG_EXMEMCNT;
    mem_setGbaCartridgeRomWaits(EXMEMCNT_SLOT2_ROM_WAIT1_10, EXMEMCNT_SLOT2_ROM_WAIT2_6);
    bool result = InitializeSdCardIntern();
    REG_EXMEMCNT = oldMemCnt;
    return result;
}

bool SuperCardLoaderPlatform::InitializeSdCardIntern()
{
    changeSupercardMode(SC_MODE_SDRAM | SC_MODE_SDCARD);
    sc_resetSdCard();
    sc_sendSdClock(NUM_STARTUP_CLOCKS);
    sc_sdCommand(SD_CMD0_GO_IDLE_STATE, 0);
    sc_sendSdClock(NUM_STARTUP_CLOCKS);

    u8 responseBuffer[17]; // purposely uninitialized
    bool isSdhc = false;
    if (sc_sdCommandAndReadResponse(SD_CMD8_SEND_IF_COND, SD_IF_COND_PATTERN, responseBuffer, SD_R1_RESPONSE_LENGTH_BYTES) &&
        responseBuffer[0] == SD_CMD8_SEND_IF_COND &&
        responseBuffer[1] == 0 &&
        responseBuffer[2] == 0 &&
        responseBuffer[3] == (SD_IF_COND_PATTERN >> 8) &&
        responseBuffer[4] == (SD_IF_COND_PATTERN & 0xFF))
    {
        isSdhc = true; //might be
    }
    {
        int i;
        for (i = 0; i < MAX_STARTUP_TRIES; ++i)
        {
            u32 arg = 0;
            arg |= (1 << 28); //Max performance
            arg |= (1 << 20); //3.3v
            if (isSdhc)
            {
                arg |= (1 << 30); // Set HCS bit,Supports SDHC
            }
            auto res = sc_sdAppCommandAndReadResponse(
                SD_ACMD41_SD_SEND_OP_COND, 0, arg, responseBuffer, SD_R1_RESPONSE_LENGTH_BYTES);
            if (res == ScAppCommandResult::FailedToSend)
            {
                return false;
            }

            if (res == ScAppCommandResult::Ok &&//ACMD41
                ((responseBuffer[1] & (1 << 7)) != 0)/*Busy:0b:initing 1b:init completed*/)
            {
                u16 ccs = responseBuffer[1] & (1 << 6);//0b:SDSC  1b:SDHC/SDXC
                if (!ccs && isSdhc)
                {
                    isSdhc = false;
                }
                break; // Card is ready
            }
            sc_sendSdClock(NUM_STARTUP_CLOCKS);
        }

        if (i >= MAX_STARTUP_TRIES)
        {
            return false;
        }
    }

    // The card's name, as assigned by the manufacturer
    sc_sdCommandAndDropResponse(SD_CMD2_ALL_SEND_CID, 0, SD_R2_RESPONSE_LENGTH_BYTES);
    // Get a new address
    u32 relativeCardAddress = 0;
    {
        int i;
        for (i = 0; i < MAX_STARTUP_TRIES; i++)
        {
            sc_sdCommandAndReadResponse(SD_CMD3_SEND_RELATIVE_ADDR, 0, responseBuffer, SD_R1_RESPONSE_LENGTH_BYTES);
            relativeCardAddress = (responseBuffer[1] << 24) | (responseBuffer[2] << 16);
            if ((responseBuffer[3] & 0x1E) != (SD_STATE_STBY << 1))
            {
                break;
            }
        }
        if (i >= MAX_STARTUP_TRIES)
        {
            return false;
        }
    }

    // Some cards won't go to higher speeds unless they think you checked their capabilities
    sc_sdCommandAndDropResponse(SD_CMD9_SEND_CSD, relativeCardAddress, SD_R2_RESPONSE_LENGTH_BYTES);

    // Only this card should respond to all future commands
    sc_sdCommandAndDropResponse(SD_CMD7_SELECT_CARD, relativeCardAddress, SD_R1_RESPONSE_LENGTH_BYTES);

    // Set a 4 bit data bus
    sc_sdAppCommandAndDropResponse(SD_ACMD6_SET_BUS_WIDTH, relativeCardAddress, 2, SD_R1_RESPONSE_LENGTH_BYTES);


    const u16 nonSdhcOpcode = THUMB_LSLS_IMM(THUMB_R1, THUMB_R0, 9);
    const u16 sdhcOpcode = THUMB_MOVS_REG(THUMB_R1, THUMB_R0);
    const u16 opcode = isSdhc ? sdhcOpcode : nonSdhcOpcode;
    scsd_writeSectorSdhcLabel = opcode;
    sclite_writeSectorSdhcLabel = opcode;
    scsd_readSectorSdhcLabel = opcode;
    sclite_readSectorSdhcLabel = opcode;

    auto type = detectSupercardType();
    switch (type)
    {
        case SUPERCARD_TYPE_SC_SD:
        case SUPERCARD_TYPE_SC_LITE:
        case SUPERCARD_TYPE_SC_RUMBLE:
        case SUPERCARD_TYPE_CHIS:
        {
            isScLite = (type & SUPERCARD_TYPE_SC_LITE) != 0;
            isSuperChis = type == SUPERCARD_TYPE_CHIS;
            return true;
        }
        default:
        {
            return false;
        }
    }
}
