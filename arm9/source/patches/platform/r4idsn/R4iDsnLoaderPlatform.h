#pragma once
#include "../acekard-common/IoRpgLoaderPlatform.h"
#include "../acekard-common/IoRpgDefinitions.h"

/// @brief Implementation of LoaderPlatform for the Acekard 2 flashcard
class R4iDsnLoaderPlatform : public IoRpgLoaderPlatform
{
private:
    enum
    {
        IORPG_CMD_SDIO_BYTE = 0xAB
    };

public:
    R4iDsnLoaderPlatform() : IoRpgLoaderPlatform(IORPG_CMD_SDIO_BYTE) { }

    const IoRpgPlatformSpecifics& GetPlatformSpecifics(void) const override
    {
        static const IoRpgPlatformSpecifics data
        {
            .cmd12Command = 0x0C0001AB,
            .cmd17Command = 0x110003AB,
            .cmd18Command = 0x120004AB,
            .cmd24Command = 0x180005AB,
            .sdStateShift = 4
        };
        return data;
    }
};
