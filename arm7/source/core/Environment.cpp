#include <nds.h>
#include "picoAgbAdapter.h"
#include "Environment.h"

#define NOCASH_ID         0x67246F6E
#define NOCASH_ID_MELONDS 0x6F6C656D

u32 Environment::_flags;

static bool detectIsNitroEmulator()
{
    u32 agbMemoryAddress = *(vu32*)0x027FFF7C;
    if (agbMemoryAddress < 0x08000000 || agbMemoryAddress >= 0x0A000000)
        return false;
    // u32 monitorRomLoadAddress = *(vu32*)0x027FFF68;
    // if (monitorRomLoadAddress < 0x02000000 || monitorRomLoadAddress >= 0x02800000)
    //     return false;
    return true;
}

static bool detectNocashPrintSupport()
{
    u32 nocashIdentifier = *(vu32*)0x04FFFA00;
    return nocashIdentifier == NOCASH_ID || nocashIdentifier == NOCASH_ID_MELONDS;
}

static bool detectPicoAgbAdapter()
{
    REG_EXMEMSTAT &= ~0xFF;
    return PICO_AGB_IDENTIFIER == PICO_AGB_IDENTIFIER_VALUE;
}

void Environment::Initialize(bool dsiMode)
{
    _flags = ENVIRONMENT_FLAGS_NONE;
    if (dsiMode)
    {
        _flags |= ENVIRONMENT_FLAGS_DSI_MODE;
    }
    else
    {
        if (detectIsNitroEmulator())
        {
            _flags |= ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR;
            _flags |= ENVIRONMENT_FLAGS_JTAG_SEMIHOSTING;

            REG_EXMEMSTAT &= ~0xFF;

            u32 agbMemoryAddress = *(vu32*)0x027FFF7C;
            if (*(vu32*)(agbMemoryAddress + 0x100) == 0x44495349) //ISID
                _flags |= ENVIRONMENT_FLAGS_AGB_SEMIHOSTING;
        }
        else
        {
            if (detectPicoAgbAdapter())
                _flags |= ENVIRONMENT_FLAGS_PICO_AGB_ADAPTER;
        }
    }
    if (!(_flags & ENVIRONMENT_FLAGS_IS_NITRO_EMULATOR))
    {
        if (detectNocashPrintSupport())
            _flags |= ENVIRONMENT_FLAGS_NOCASH_PRINT;
    }
}
