#pragma once
#include "common.h"
#include "SuperCardDefinitions.h"

enum class ScAppCommandResult
{
    Ok,
    FailedToSend,
    FailedToParseResponse,
};

void sc_sdCommand(u8 command, u32 argument);
void sc_sdCommandAndDropResponse(u8 command, u32 argument, u32 bytesToDrop);
bool sc_sdCommandAndReadResponse(u8 command, u32 argument, u8* responseBuffer, u32 bytesToRead);

ScAppCommandResult sc_sdAppCommand(u8 appCommand, u32 relativeCardAddress, u32 argument);
ScAppCommandResult sc_sdAppCommandAndDropResponse(u8 appCommand,
    u32 relativeCardAddress, u32 argument, u32 bytesToDrop);
ScAppCommandResult sc_sdAppCommandAndReadResponse(u8 appCommand,
    u32 relativeCardAddress, u32 argument, u8* responseBuffer, u32 bytesToRead);

static inline void sc_dummyRead(auto) { }

static inline void sc_sendSdClock(u32 num)
{
    while (num--)
    {
        sc_dummyRead(REG_SCSD_CMD16);
    }
}

static inline void sc_resetSdCard()
{
    SD_RESET_ADDR = 0;
}
