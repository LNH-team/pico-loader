#include "common.h"
#include "M3CFLoaderPlatform.h"

#define M3_MODE_ROM 8
#define M3_MODE_MEDIA 6

static u16 M3_readHalfword(u32 addr)
{
	return *((vu16*)addr);
}

static void M3_changeMode(u32 mode)
{
	M3_readHalfword(0x08e00002);
	M3_readHalfword(0x0800000e);
	M3_readHalfword(0x08801ffc);
	M3_readHalfword(0x0800104a);
	M3_readHalfword(0x08800612);
	M3_readHalfword(0x08000000);
	M3_readHalfword(0x08801b66);
	M3_readHalfword(0x08800000 + mode);
	M3_readHalfword(0x0800080e);
	M3_readHalfword(0x08000000);
	
	if(mode != M3_MODE_ROM) {
		M3_readHalfword(0x09000000);
	} else {
		M3_readHalfword(0x080001e4);
		M3_readHalfword(0x080001e4);
		M3_readHalfword(0x08000188);
		M3_readHalfword(0x08000188);
	}
}

void M3CFLoaderPlatform::CardLockUnlock(bool lock) const
{
    M3_changeMode(lock ? M3_MODE_ROM : M3_MODE_MEDIA);
}
