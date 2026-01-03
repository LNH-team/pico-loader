#include "common.h"
#include <libtwl/card/card.h>
#include "R4LoaderPlatform.h"

#define R4_CMD_CARD_INFO    0xB000000000000000ull
#define R4_CMD_SEND_MAP     0xB400000000000000ull

u32 R4LoaderPlatform::ReadCardInfo() const
{
    u32 response;
    card_romSetCmd(R4_CMD_CARD_INFO);
    card_romStartXfer(MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE |
        MCCNT1_CLOCK_SCRAMBLER | MCCNT1_READ_DATA_DESCRAMBLE |
        MCCNT1_LATENCY2(24) | MCCNT1_LATENCY1(0) | MCCNT1_LEN_4, false);
    card_romCpuRead(&response, 1);
    return response;
}

void R4LoaderPlatform::PrepareClusterMap(bool isSave, u32 dirSector, u32 dirSectorOffset) const
{
    u32 fatEntryAddress = ((dirSector << 9) | dirSectorOffset) | (isSave ? 1 : 0);
    card_romSetCmd(R4_CMD_SEND_MAP | ((u64)fatEntryAddress << 24));
    u32 response;
    do
    {
        card_romStartXfer(MCCNT1_RESET_OFF | MCCNT1_CMD_SCRAMBLE |
            MCCNT1_CLOCK_SCRAMBLER | MCCNT1_READ_DATA_DESCRAMBLE |
            MCCNT1_LATENCY2(24) | MCCNT1_LATENCY1(0) | MCCNT1_LEN_4, false);
        card_romCpuRead(&response, 1);
    } while (response);
}

void R4LoaderPlatform::PrepareRomBoot(u32 romDirSector, u32 romDirSectorOffset) const
{
    ReadCardInfo();
    if (romDirSector != 0)
    {
        PrepareClusterMap(false, romDirSector, romDirSectorOffset);
    }
    ReadCardInfo();
}