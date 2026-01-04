#pragma once
#include <nds/ndstypes.h>

struct nds_header_ntr_t
{
    u8 gameTitle[12];
    u32 gameCode;
    u8 makerCode[2];
    u8 unitCode;
    u8 encryptionSeedSelect;
    u8 deviceCapacity;
    u8 gap15[7];
    u8 twlFlags;
    u8 flags;
    u8 softwareVersion;
    u8 flags2;
    u32 arm9RomOffset;
    u32 arm9EntryAddress;
    u32 arm9LoadAddress;
    u32 arm9Size;
    u32 arm7RomOffset;
    u32 arm7EntryAddress;
    u32 arm7LoadAddress;
    u32 arm7Size;
    u32 fntOffset;
    u32 fntSize;
    u32 fatOffset;
    u32 fatSize;
    u32 arm9OverlayTableOffset;
    u32 arm9OverlayTableSize;
    u32 arm7OverlayTableOffset;
    u32 arm7OverlayTableSize;
    u32 normalModeRomControl;
    u32 secureModeRomControl;
    u32 bannerOffset;
    u16 secureAreaCrc;
    u16 secureDelay;
    u32 arm9AutoLoadDoneHookAddress;
    u32 arm7AutoLoadDoneHookAddress;
    u8 secureAreaDisable[8];
    u32 ntrRomSize;
    u32 romHeaderSize;
    u32 arm9ModuleParamsAddress;
    u32 arm7ModuleParamsAddress;
    u16 ntrRegionEnd;
    u16 twlRegionStart;
    u16 nandRomRegionEnd;
    u16 nandBackupRegionStart;
    u8 gap98[0x28];
    u8 nintendoLogo[0x9C];
    u16 nintendoLogoCrc;
    u16 headerCrc;
    u32 debugRomOffset;
    u32 debugRomSize;
    u32 debugRomLoadAddress; // and arm9 entry
    u32 debugRomArm7EntryAddress;

    constexpr bool IsTwlRom() const
    {
        return unitCode & 2;
    }
};

static_assert(sizeof(nds_header_ntr_t) == 0x170, "Invalid size for nds_header_ntr_t");

#define NDS_HEADER_TWL_ACCESS_CONTROL_SD_ACCESS     (1 << 3)
#define NDS_HEADER_TWL_ACCESS_CONTROL_NAND_ACCESS   (1 << 4)

struct nds_header_twl_t : public nds_header_ntr_t
{
    u8 gap170[0x10];
    u32 globalMbkSettings[5]; // slot mapping
    u32 arm9MbkSettings[3]; // wram mapping
    u32 arm7MbkSettings[3]; // wram mapping
    u8 mbk9Setting[3];
    u8 wramCntSetting;
    u32 regionFlags;
    u32 accessControl;
    u32 arm7ScfgExt7;
    u8 gap1B8[3];
    u8 twlFlags2;
    u32 arm9iRomOffset;
    u8 gap1C4[4];
    u32 arm9iLoadAddress;
    u32 arm9iSize;
    u32 arm7iRomOffset;
    u32 arm7DeviceListAddress;
    u32 arm7iLoadAddress;
    u32 arm7iSize;
    u32 ntrDigestRegionOffset;
    u32 ntrDigestRegionSize;
    u32 twlDigestRegionOffset;
    u32 twlDigestRegionSize;
    u32 digestSectorHashtableOffset;
    u32 digestSectorHashtableSize;
    u32 digestBlockHashtableOffset;
    u32 digestBlockHashtableSize;
    u32 digestSectorSize;
    u32 digestSectorsPerBlock;
    u32 bannerSize;
    u8 shared2File0Size;
    u8 shared2File1Size;
    u8 eulaVersion;
    u8 twlManagementFlags;
    u32 totalTwlRomSize; // can be 0
    u8 shared2File2Size;
    u8 shared2File3Size;
    u8 shared2File4Size;
    u8 shared2File5Size;
    u32 arm9iModuleParamsAddress;
    u32 arm7iModuleParamsAddress;
    u32 modcryptArea1Offset;
    u32 modcryptArea1Size;
    u32 modcryptArea2Offset;
    u32 modcryptArea2Size;
    u64 titleId;
    u32 twlPublicSavSize;
    u32 twlPrivateSavSize;
    u8 gap240[0xB0];
    u8 parentalControlRatings[0x10];
    u8 arm9Sha1Hmac[0x14];
    u8 arm7Sha1Hmac[0x14];
    u8 digestMasterSha1Hmac[0x14];
    u8 bannerSha1Hmac[0x14];
    u8 arm9iSha1Hmac[0x14];
    u8 arm7iSha1Hmac[0x14];
    u8 ntrHeaderArm9Arm7Sha1Hmac[0x14]; // whitelist phase 1
    u8 ntrArm9OvtOverlaysFatSha1Hmac[0x14]; // whitelist phase 2
    u8 arm9WithoutSecureAreaSha1Hmac[0x14];
    u8 gap3B4[0xA4C];
    u8 debugArguments[0x100];
    u8 gapF00[0x80];
    u8 headerRsaSha1Signature[0x80];

    constexpr bool IsDsiWare() const
    {
        return IsTwlRom() && ((titleId >> 32) & 4) != 0;
    }

    constexpr bool HasSdAccess() const
    {
        return IsTwlRom() && (accessControl & NDS_HEADER_TWL_ACCESS_CONTROL_SD_ACCESS) != 0;
    }

    constexpr bool HasNandAccess() const
    {
        return IsTwlRom() && (accessControl & NDS_HEADER_TWL_ACCESS_CONTROL_NAND_ACCESS) != 0;
    }
};

static_assert(sizeof(nds_header_twl_t) == 0x1000, "Invalid size for nds_header_twl_t");
