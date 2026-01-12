#include "common.h"
#include <string.h>
#include <algorithm>
#include <memory>
#include "DsiWareSaveArranger.h"

bool DsiWareSaveArranger::SetupDsiWareSave(const TCHAR* romPath, const nds_header_twl_t& romHeader, DsiWareSaveResult& result) const
{
    char path[256];
    strcpy(path, romPath);
    if (!CreateDeviceListPath(path, result.romFilePath))
    {
        return false;
    }

    if (romHeader.twlPrivateSavSize != 0)
    {
        char* extension = strrchr(path, '.');
        if (!extension)
            extension = &path[strlen(path)];
        extension[0] = '.';
        extension[1] = 'p';
        extension[2] = 'r';
        extension[3] = 'v';
        extension[4] = 0;
        if (!SetupDsiWareSaveFile(path, romHeader.twlPrivateSavSize) ||
            !CreateDeviceListPath(path, result.privateSavePath))
        {
            return false;
        }
    }

    if (romHeader.twlPublicSavSize != 0)
    {
        strcpy(path, romPath);
        char* extension = strrchr(path, '.');
        if (!extension)
            extension = &path[strlen(path)];
        extension[0] = '.';
        extension[1] = 'p';
        extension[2] = 'u';
        extension[3] = 'b';
        extension[4] = 0;
        if (!SetupDsiWareSaveFile(path, romHeader.twlPublicSavSize) ||
            !CreateDeviceListPath(path, result.publicSavePath))
        {
            return false;
        }
    }

    return true;
}

bool DsiWareSaveArranger::SetupDsiWareSaveFile(const TCHAR* savePath, u32 saveSize) const
{
    if (saveSize == 0)
    {
        return true;
    }

    auto file = std::make_unique<FIL>();
    if (f_open(file.get(), savePath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
    {
        LOG_FATAL("Failed to open or create save file\n");
        return false;
    }

    u32 initialSize = f_size(file.get());
    if (initialSize < saveSize)
    {
        if (f_lseek(file.get(), saveSize) != FR_OK ||
            f_lseek(file.get(), 0) != FR_OK)
        {
            LOG_FATAL("Failed to create DSiWare save file\n");
            return false;
        }

        auto fatHeader = CreateFatHeader(saveSize);

        UINT bytesWritten = 0;
        if (f_write(file.get(), fatHeader.get(), sizeof(fat_header_t), &bytesWritten) != FR_OK ||
            bytesWritten != sizeof(fat_header_t))
        {
            LOG_FATAL("Failed to format DSiWare save file\n");
            return false;
        }

        fatHeader.reset();

        const u32 clearBufferSize = 32 * 1024;
        auto clearBuffer = std::make_unique<u8[]>(clearBufferSize);
        memset(clearBuffer.get(), 0, clearBufferSize);

        u32 offset = f_tell(file.get());
        while (offset < saveSize)
        {
            u32 bytesToWrite = std::min<u32>(saveSize - offset, clearBufferSize);
            bytesWritten = 0;
            if (f_write(file.get(), clearBuffer.get(), bytesToWrite, &bytesWritten) != FR_OK ||
                bytesWritten != bytesToWrite)
            {
                LOG_FATAL("Failed to format DSiWare save file\n");
                return false;
            }
            offset += bytesToWrite;
        }
    }

    f_close(file.get());
    return true;
}

std::unique_ptr<DsiWareSaveArranger::fat_header_t> DsiWareSaveArranger::CreateFatHeader(u32 saveSize) const
{
    // based on https://github.com/Epicpkmn11/NTM/blob/master/arm9/src/sav.c
    const u32 maxSectors = saveSize >> 9;
    u32 sectorCount = 1;
    u32 secPerTrk = 1;
    u32 numHeads = 1;
    u32 sectorCountNext = 0;
    while (sectorCountNext <= maxSectors)
    {
        sectorCountNext = secPerTrk * (numHeads + 1) * (numHeads + 1);
        if (sectorCountNext <= maxSectors)
        {
            numHeads++;
            sectorCount = sectorCountNext;

            secPerTrk++;
            sectorCountNext = secPerTrk * numHeads * numHeads;
            if (sectorCountNext <= maxSectors)
            {
                sectorCount = sectorCountNext;
            }
        }
    }
    sectorCountNext = (secPerTrk + 1) * numHeads * numHeads;
    if (sectorCountNext <= maxSectors)
    {
        secPerTrk++;
        sectorCount = sectorCountNext;
    }

    u32 sectorsPerCluster;
    u32 totalClusters;
    if (sectorCount > 8192)
    {
        sectorsPerCluster = 8;
        totalClusters = (sectorCount + 7) >> 3;
    }
    else if (sectorCount > 1024)
    {
        sectorsPerCluster = 4;
        totalClusters = (sectorCount + 3) >> 2;
    }
    else
    {
        sectorsPerCluster = 1;
        totalClusters = sectorCount;
    }

    u32 fatSizeInBytes = ((totalClusters + 1) >> 1) * 3; // 2 sectors -> 3 byte

    auto fatHeader = std::make_unique<fat_header_t>();

    fatHeader->jumpInstruction[0] = 0xE9;
    fatHeader->jumpInstruction[1] = 0;
    fatHeader->jumpInstruction[2] = 0;
    memcpy(fatHeader->oemName, "MSWIN4.1", 8);
    fatHeader->bytesPerSector = 512;
    fatHeader->sectorsPerCluster = sectorsPerCluster;
    fatHeader->reservedSectors = 1;
    fatHeader->numberOfFats = 2;
    fatHeader->rootEntries = saveSize < 0x8C000 ? 32 : 512;
    fatHeader->totalSectorsSmall = sectorCount;
    fatHeader->mediaType = 0xF8; // "hard drive"
    fatHeader->fatSectorCount = (fatSizeInBytes + 511) >> 9;
    fatHeader->sectorsPerTrack = secPerTrk;
    fatHeader->numberOfHeads = numHeads;
    fatHeader->hiddenSectors = 0;
    fatHeader->totalSectorsLarge = 0;
    fatHeader->diskNumber = 5;
    fatHeader->BS_Reserved1 = 0;
    fatHeader->bootSignature = 0x29;
    fatHeader->volumeSerialNumber = 0x12345678;
    memcpy(fatHeader->volumeLabel, "VOLUMELABEL", 11);
    memcpy(fatHeader->fileSystemType, "FAT12   ", 8);
    memset(fatHeader->bootCode, 0, sizeof(fatHeader->bootCode));
    fatHeader->endOfSectorMarker = 0xAA55;

    return fatHeader;
}

bool DsiWareSaveArranger::CreateDeviceListPath(TCHAR* savePath, char* deviceListPath) const
{
    auto fileInfo = std::make_unique<FILINFO>();
    strcpy(deviceListPath, "sdmc:/");
    char* shortPath = deviceListPath + 6;
    char* currentPathSegment = strchr(savePath, '/');
    do
    {
        currentPathSegment = strchr(currentPathSegment + 1, '/');
        if (currentPathSegment)
        {
            *currentPathSegment = 0;
        }

        if (f_stat(savePath, fileInfo.get()) != FR_OK)
        {
            LOG_DEBUG("Failed to create short path\n");
            return false;
        }

        LOG_DEBUG("%s\n", savePath);
        // Use fname when altname is empty to handle short filenames.
        const char* nameToUse = fileInfo->altname[0]
            ? fileInfo->altname
            : fileInfo->fname;
        u32 length = strlen(nameToUse);
        if (shortPath + length - deviceListPath >= 64)
        {
            LOG_DEBUG("Path too long\n");
            return false;
        }

        strcpy(shortPath, nameToUse);
        shortPath += length;
        if (currentPathSegment)
        {
            if (shortPath + 1 - deviceListPath >= 64)
            {
                LOG_DEBUG("Path too long\n");
                return false;
            }

            *shortPath++ = '/';
            *currentPathSegment = '/';
        }
    } while (currentPathSegment);
    LOG_DEBUG("%s\n", deviceListPath);

    return true;
}
