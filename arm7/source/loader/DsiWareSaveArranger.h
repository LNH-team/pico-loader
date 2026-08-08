#pragma once
#include <memory>
#include "fat/ff.h"
#include "ndsHeader.h"

/// @brief Struct holding the result of setting up DSiWare save data.
struct DsiWareSaveResult
{
    /// @brief The short filename path of the private save file.
    char privateSavePath[64];

    /// @brief The short filename path of the public save file.
    char publicSavePath[64];

    /// @brief The short filename path of rom file.
    char romFilePath[64];

    /// @brief The short filename path of banner save file.
    char bannerSaveFilePath[64];
};

/// @brief Class for setting up the save files for DSiWare roms.
class DsiWareSaveArranger
{
public:
    /// @brief Sets up the save files for a DSiWare rom with the given \p romPath and \p romHeader.
    /// @param romPath The path to the DSiWare rom to setup the save files for.
    /// @param romHeader The header of the DSiWare rom.
    /// @param result Struct in which the resulting paths are returned.
    /// @return \c true when setting up the save was successful, or \c false otherwise.
    bool SetupDsiWareSave(const TCHAR* romPath, const nds_header_twl_t& romHeader, DsiWareSaveResult& result) const;

private:
#pragma pack(push, 1)
    /// @brief Struct representing a FAT header.
    struct fat_header_t
    {
        u8 jumpInstruction[3];
        u8 oemName[8];
        u16 bytesPerSector;
        u8 sectorsPerCluster;
        u16 reservedSectors;
        u8 numberOfFats;
        u16 rootEntries;
        u16 totalSectorsSmall;
        u8 mediaType;
        u16 fatSectorCount;
        u16 sectorsPerTrack;
        u16 numberOfHeads;
        u32 hiddenSectors;
        u32 totalSectorsLarge;
        u8 diskNumber;
        u8 BS_Reserved1;
        u8 bootSignature;
        u32 volumeSerialNumber;
        u8 volumeLabel[11];
        u8 fileSystemType[8];
        u8 bootCode[448];
        u16 endOfSectorMarker;
    };
#pragma pack(pop)

    static_assert(sizeof(fat_header_t) == 512, "Invalid size of fat_header_t.");

    bool SetupDsiWareSaveFile(const TCHAR* savePath, u32 saveSize) const;
    std::unique_ptr<fat_header_t> CreateFatHeader(u32 saveSize) const;
    bool CreateDeviceListPath(TCHAR* savePath, char* deviceListPath) const;
    bool CreateBannerSave(TCHAR* bannerSavePath) const;
};
