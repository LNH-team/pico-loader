#include "common.h"
#include "../SdioDefinitions.h"
#include "SuperCardSdCommands.h"
#include "SuperCardDefinitions.h"

#define BUSY_WAIT_TIMEOUT 500000

static const u8 sCrc7Lut[]
{
    0x0, 0x12, 0x24, 0x36, 0x48, 0x5A, 0x6C, 0x7E, 0x90, 0x82, 0xB4, 0xA6, 0xD8, 0xCA, 0xFC, 0xEE,
    0x32, 0x20, 0x16, 0x4, 0x7A, 0x68, 0x5E, 0x4C, 0xA2, 0xB0, 0x86, 0x94, 0xEA, 0xF8, 0xCE, 0xDC,
    0x64, 0x76, 0x40, 0x52, 0x2C, 0x3E, 0x8, 0x1A, 0xF4, 0xE6, 0xD0, 0xC2, 0xBC, 0xAE, 0x98, 0x8A,
    0x56, 0x44, 0x72, 0x60, 0x1E, 0xC, 0x3A, 0x28, 0xC6, 0xD4, 0xE2, 0xF0, 0x8E, 0x9C, 0xAA, 0xB8,
    0xC8, 0xDA, 0xEC, 0xFE, 0x80, 0x92, 0xA4, 0xB6, 0x58, 0x4A, 0x7C, 0x6E, 0x10, 0x2, 0x34, 0x26,
    0xFA, 0xE8, 0xDE, 0xCC, 0xB2, 0xA0, 0x96, 0x84, 0x6A, 0x78, 0x4E, 0x5C, 0x22, 0x30, 0x6, 0x14,
    0xAC, 0xBE, 0x88, 0x9A, 0xE4, 0xF6, 0xC0, 0xD2, 0x3C, 0x2E, 0x18, 0xA, 0x74, 0x66, 0x50, 0x42,
    0x9E, 0x8C, 0xBA, 0xA8, 0xD6, 0xC4, 0xF2, 0xE0, 0xE, 0x1C, 0x2A, 0x38, 0x46, 0x54, 0x62, 0x70,
    0x82, 0x90, 0xA6, 0xB4, 0xCA, 0xD8, 0xEE, 0xFC, 0x12, 0x0, 0x36, 0x24, 0x5A, 0x48, 0x7E, 0x6C,
    0xB0, 0xA2, 0x94, 0x86, 0xF8, 0xEA, 0xDC, 0xCE, 0x20, 0x32, 0x4, 0x16, 0x68, 0x7A, 0x4C, 0x5E,
    0xE6, 0xF4, 0xC2, 0xD0, 0xAE, 0xBC, 0x8A, 0x98, 0x76, 0x64, 0x52, 0x40, 0x3E, 0x2C, 0x1A, 0x8,
    0xD4, 0xC6, 0xF0, 0xE2, 0x9C, 0x8E, 0xB8, 0xAA, 0x44, 0x56, 0x60, 0x72, 0xC, 0x1E, 0x28, 0x3A,
    0x4A, 0x58, 0x6E, 0x7C, 0x2, 0x10, 0x26, 0x34, 0xDA, 0xC8, 0xFE, 0xEC, 0x92, 0x80, 0xB6, 0xA4,
    0x78, 0x6A, 0x5C, 0x4E, 0x30, 0x22, 0x14, 0x6, 0xE8, 0xFA, 0xCC, 0xDE, 0xA0, 0xB2, 0x84, 0x96,
    0x2E, 0x3C, 0xA, 0x18, 0x66, 0x74, 0x42, 0x50, 0xBE, 0xAC, 0x9A, 0x88, 0xF6, 0xE4, 0xD2, 0xC0,
    0x1C, 0xE, 0x38, 0x2A, 0x54, 0x46, 0x70, 0x62, 0x8C, 0x9E, 0xA8, 0xBA, 0xC4, 0xD6, 0xE0, 0xF2
};

static inline void dropResponse(int bytesToDrop)
{
    bytesToDrop++; // +  8 clocks

    // Wait for the card to be non-busy
    while ((REG_SCSD_CMD16 & 1) != 0);

    while (--bytesToDrop)
    {
        sc_dummyRead(REG_SCSD_CMD32);
        sc_dummyRead(REG_SCSD_CMD32);
        sc_dummyRead(REG_SCSD_CMD32);
        sc_dummyRead(REG_SCSD_CMD32);
    }
}

static inline bool readResponse(u8* dest, u32 length)
{
    for (u32 i = BUSY_WAIT_TIMEOUT; (REG_SCSD_CMD16 & 1) != 0; --i)
    {
        if (i == 0)
        {
            return false;
        }
    }

    int numBits = length * 8;
    // The first bit is always 0
    u32 partialResult = (REG_SCSD_CMD16 & 1) << 16;
    numBits -= 2;
    // Read the remaining bits in the response.
    // It's always most significant bit first
    const u32 mask2Bit = 0x10001;
    while (numBits)
    {
        numBits-=2;
        partialResult = (partialResult << 2) | (REG_SCSD_CMD32 & mask2Bit);
        if ((numBits & 7) == 0)
        {
            //_1_3_5_7 _0_2_4_6
            *dest++ = ((partialResult >> 16) | (partialResult<<1));
            partialResult = 0;
        }
    }
    for (int i = 0; i < 4; ++i) //8clock
    {
        sc_dummyRead(REG_SCSD_CMD32);
    }

    return true;
}

static inline u8 crc7One(u8 crcIn, u8 data)
{
    crcIn ^= data;
    return sCrc7Lut[crcIn];
}

static inline u8 sdCrc7(u8 *pBuf, int len)
{
    u8 crc = 0;
    while (len--)
    {
        crc = crc7One(crc, *pBuf++);
    }
    crc |= 1;
    return crc;
}

void sc_sdCommand(u8 command, u32 argument)
{
    u8 databuff[6];
    u8* tempDataPtr = databuff;

    *tempDataPtr++ = command | 0x40;
    *tempDataPtr++ = argument >> 24;
    *tempDataPtr++ = argument >> 16;
    *tempDataPtr++ = argument >> 8;
    *tempDataPtr++ = argument;
    *tempDataPtr = sdCrc7(databuff, 5);

    while ((REG_SCSD_CMD16 & 1) == 0);

    sc_dummyRead(REG_SCSD_CMD16);

    auto* sendCommandAddr = &REG_SCSD_CMD32;
    for (u32 data : databuff)
    {
        data |= data << 17;
        *sendCommandAddr++ = data;
        *sendCommandAddr++ = data << 2;
        *sendCommandAddr++ = data << 4;
        *sendCommandAddr++ = data << 6;
    }
}

void sc_sdCommandAndDropResponse(u8 command, u32 argument, u32 bytesToDrop)
{
    sc_sdCommand(command, argument);
    dropResponse(bytesToDrop);
}

bool sc_sdCommandAndReadResponse(u8 command, u32 argument, u8* responseBuffer, u32 bytesToRead)
{
    sc_sdCommand(command, argument);
    return readResponse(responseBuffer, bytesToRead);
}

ScAppCommandResult sc_sdAppCommand(u8 appCommand, u32 relativeCardAddress, u32 argument)
{
    u8 responseBuffer[6]; // purposely uninitialized
    sc_sdCommandAndReadResponse(SD_CMD55_APP_CMD, relativeCardAddress, responseBuffer, 6);
    if (responseBuffer[0] == SD_CMD55_APP_CMD)
    {
        sc_sdCommand(appCommand, argument);
        return ScAppCommandResult::Ok;
    }
    else
    {
        return ScAppCommandResult::FailedToSend;
    }
}

ScAppCommandResult sc_sdAppCommandAndDropResponse(u8 appCommand,
    u32 relativeCardAddress, u32 argument, u32 bytesToDrop)
{
    auto res = sc_sdAppCommand(appCommand, relativeCardAddress, argument);
    if (res == ScAppCommandResult::Ok)
    {
        dropResponse(bytesToDrop);
    }

    return res;
}

ScAppCommandResult sc_sdAppCommandAndReadResponse(u8 appCommand,
    u32 relativeCardAddress, u32 argument, u8* responseBuffer, u32 bytesToRead)
{
    auto res = sc_sdAppCommand(appCommand, relativeCardAddress, argument);
    if (res == ScAppCommandResult::Ok)
    {
        res = readResponse(responseBuffer, bytesToRead)
            ? ScAppCommandResult::Ok
            : ScAppCommandResult::FailedToParseResponse;
    }

    return res;
}
