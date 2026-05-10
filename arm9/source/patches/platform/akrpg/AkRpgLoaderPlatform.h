#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "../acekard-common/IoRpgDefinitions.h"

/// @brief Implementation of LoaderPlatform for the Acekard RPG flashcard
class AkRpgLoaderPlatform : public IoRpgLoaderPlatform
{
private:
    enum
    {
        IORPG_CMD_SDIO_BYTE = 0xD5
    };

public:
    AkRpgLoaderPlatform() : IoRpgLoaderPlatform(IORPG_CMD_SDIO_BYTE) { }

    const IoRpgPlatformSpecifics& GetPlatformSpecifics(void) const override
    {
        static const IoRpgPlatformSpecifics data
        {
            .cmd12Command = 0x0C0001D5,
            .cmd17Command = 0x110003D5,
            .cmd18Command = 0x120004D5,
            .cmd24Command = 0x180005D5,
            .sdStateShift = 4
        };
        return data;
    }
};
