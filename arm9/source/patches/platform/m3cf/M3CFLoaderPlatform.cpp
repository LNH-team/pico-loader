#include "common.h"
#include "M3CFLoaderPlatform.h"

#define M3_MODE_ROM 8
#define M3_MODE_MEDIA 6

static u16 volatileReadHalfword(u32 addr)
{
    return *((vu16*)addr);
}

static void changeM3Mode(u32 mode)
{
    volatileReadHalfword(0x08e00002);
    volatileReadHalfword(0x0800000e);
    volatileReadHalfword(0x08801FFC);
    volatileReadHalfword(0x0800104A);
    volatileReadHalfword(0x08800612);
    volatileReadHalfword(0x08000000);
    volatileReadHalfword(0x08801B66);
    volatileReadHalfword(0x08800000 + mode);
    volatileReadHalfword(0x0800080E);
    volatileReadHalfword(0x08000000);
    
    if(mode == M3_MODE_ROM)
    {
        volatileReadHalfword(0x080001E4);
        volatileReadHalfword(0x080001E4);
        volatileReadHalfword(0x08000188);
        volatileReadHalfword(0x08000188);
    }
    else
    {
        volatileReadHalfword(0x09000000);
    }
}

void M3CFLoaderPlatform::SetCardLocked(bool locked) const
{
    changeM3Mode(locked ? M3_MODE_ROM : M3_MODE_MEDIA);
}
