#include "common.h"
#include "M3CFLoaderPlatform.h"

#define M3_MODE_ROM 8
#define M3_MODE_MEDIA 6

static u16 M3_readHalfword(u32 addr)
{
    return *((vu16*)addr);
}

static void changeM3Mode(u32 mode)
{
    M3_readHalfword(0x08e00002);
    M3_readHalfword(0x0800000e);
    M3_readHalfword(0x08801FFC);
    M3_readHalfword(0x0800104A);
    M3_readHalfword(0x08800612);
    M3_readHalfword(0x08000000);
    M3_readHalfword(0x08801B66);
    M3_readHalfword(0x08800000 + mode);
    M3_readHalfword(0x0800080E);
    M3_readHalfword(0x08000000);
    
    if(mode == M3_MODE_ROM)
    {
        M3_readHalfword(0x080001E4);
        M3_readHalfword(0x080001E4);
        M3_readHalfword(0x08000188);
        M3_readHalfword(0x08000188);
    }
    else
    {
        M3_readHalfword(0x09000000);
    }
}

void M3CFLoaderPlatform::SetCardLocked(bool locked) const
{
    changeM3Mode(locked ? M3_MODE_ROM : M3_MODE_MEDIA);
}
