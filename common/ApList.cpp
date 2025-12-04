#include "common.h"
#include <algorithm>
#include "ApList.h"

const ApListEntry* ApList::FindEntry(u32 gameCode, u8 gameVersion)
{
    if (_count != 0)
    {
        const auto gameEntry = std::lower_bound(_entries.get(), _entries.get() + _count, gameCode,
            [gameVersion] (const ApListEntry& entry, u32 value)
            {
                if (entry.GetGameCode() == value)
                    return entry.GetGameVersion() < gameVersion;
                return entry.GetGameCode() < value;
            });

        if (gameEntry != _entries.get() + _count &&
            gameEntry->GetGameCode() == gameCode &&
            gameEntry->GetGameVersion() == gameVersion)
        {
            return gameEntry;
        }
    }

    return nullptr;
}

void ApListEntry::Dump() const
{
    const char* dsProtectVersionString;
    switch (GetDSProtectVersion())
    {
        case DSProtectVersion::v1_00_2: dsProtectVersionString = "v1.00_2"; break;
        case DSProtectVersion::v1_05: dsProtectVersionString = "v1.05"; break;
        case DSProtectVersion::v1_06: dsProtectVersionString = "v1.06"; break;
        case DSProtectVersion::v1_08: dsProtectVersionString = "v1.08"; break;
        case DSProtectVersion::v1_10: dsProtectVersionString = "v1.10"; break;
        case DSProtectVersion::v1_20: dsProtectVersionString = "v1.20"; break;
        case DSProtectVersion::v1_22: dsProtectVersionString = "v1.22"; break;
        case DSProtectVersion::v1_23: dsProtectVersionString = "v1.23"; break;
        case DSProtectVersion::v1_23Z: dsProtectVersionString = "v1.23Z"; break;
        case DSProtectVersion::v1_25: dsProtectVersionString = "v1.25"; break;
        case DSProtectVersion::v1_26: dsProtectVersionString = "v1.26"; break;
        case DSProtectVersion::v1_27: dsProtectVersionString = "v1.27"; break;
        case DSProtectVersion::v1_28: dsProtectVersionString = "v1.28"; break;
        case DSProtectVersion::v2_00: dsProtectVersionString = "v2.00"; break;
        case DSProtectVersion::v2_01: dsProtectVersionString = "v2.01"; break;
        case DSProtectVersion::v2_03: dsProtectVersionString = "v2.03"; break;
        case DSProtectVersion::v2_05: dsProtectVersionString = "v2.05"; break;
        case DSProtectVersion::v2_00s: dsProtectVersionString = "v2.00s"; break;
        case DSProtectVersion::v2_01s: dsProtectVersionString = "v2.01s"; break;
        case DSProtectVersion::v2_03s: dsProtectVersionString = "v2.03s"; break;
        case DSProtectVersion::v2_05s: dsProtectVersionString = "v2.05s"; break;
        default:
            dsProtectVersionString = "unknown";
            break;
    }
    LOG_DEBUG("%c%c%c%c - %d - DSProtect %s\n",
        gameCode & 0xFF, (gameCode >> 8) & 0xFF, (gameCode >> 16) & 0xFF, gameCode >> 24,
        gameVersion,
        dsProtectVersionString);
    if (regularOverlayId != AP_LIST_OVERLAY_ID_INVALID)
    {
        if (regularOverlayId == AP_LIST_OVERLAY_ID_STATIC_ARM9)
        {
            LOG_DEBUG("regular: main memory offset 0x%x\n", GetRegularOffset());
        }
        else
        {
            LOG_DEBUG("regular: overlay %d offset 0x%x\n", regularOverlayId, GetRegularOffset());
        }
    }
    if (sOverlayId != AP_LIST_OVERLAY_ID_INVALID)
    {
        if (sOverlayId == AP_LIST_OVERLAY_ID_STATIC_ARM9)
        {
            LOG_DEBUG("s: main memory offset 0x%x\n", GetSOffset());
        }
        else
        {
            LOG_DEBUG("s: overlay %d offset 0x%x\n", sOverlayId, GetSOffset());
        }
    }
}