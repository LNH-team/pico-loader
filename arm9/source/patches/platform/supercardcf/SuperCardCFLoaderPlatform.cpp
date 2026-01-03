#include "common.h"
#include "SuperCardCFLoaderPlatform.h"

#define SC_MODE_REG         (*(vu16*)0x09FFFFFE)
#define SC_MODE_MAGIC       0xA55A

#define SC_MODE_MEDIA       0x3
#define SC_MODE_RAM_RO      0x1

static void changeSupercardMode(u8 mode)
{
    SC_MODE_REG = SC_MODE_MAGIC;
    SC_MODE_REG = SC_MODE_MAGIC;
    SC_MODE_REG = mode;
    SC_MODE_REG = mode;
}

void SuperCardCFLoaderPlatform::SetCardLocked(bool locked) const
{
    changeSupercardMode(locked ? SC_MODE_RAM_RO : SC_MODE_MEDIA);
}
