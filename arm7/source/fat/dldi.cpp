#include "common.h"
#include <memory>
#include <malloc.h>
#include <string.h>
#include "ipc.h"
#include "ipcCommands.h"
#include "loader/DldiDriver.h"
#include "dldi.h"

#define DLDI_CACHE_PATH         "/_pico/dldi.bin"
#define DLDI_CACHE_TEMP_PATH    "/_pico/dldi.tmp"

static u8 sDldiBuffer[16 * 1024] alignas(32);
static DldiDriver sDldiDriver = DldiDriver((dldi_header_t*)sDldiBuffer);

/// @brief Whether the driver in sDldiBuffer was handed down through gLoaderHeader.dldiDriver.
///        Otherwise sDldiBuffer only holds entry points into the platform's SD patch code,
///        which can be used for Pico Loader's own reads but cannot be patched into a program.
static bool sIsHandedDownDriver = false;

/// @brief The card's own driver loaded from DLDI_CACHE_PATH, or \c nullptr when none was loaded.
static dldi_header_t* sCachedDriver = nullptr;

[[gnu::target("thumb")]]
static bool readSectorsWithPatchCode(u32 sector, u32 count, void* buffer)
{
    typedef void (*patch_code_read_sd_sectors_t)(u32 srcSector, void* dst, u32 sectorCount);
    (*(patch_code_read_sd_sectors_t*)0x037F8000)(sector, buffer, count);
    return true;
}

[[gnu::target("thumb")]]
static bool writeSectorsWithPatchCode(u32 sector, u32 count, const void* buffer)
{
    typedef void (*patch_code_write_sd_sectors_t)(u32 dstSector, const void* src, u32 sectorCount);
    (*(patch_code_write_sd_sectors_t*)0x037F8004)(sector, buffer, count);
    return true;
}

bool dldi_init()
{
    auto driver = (const dldi_header_t*)gLoaderHeader.dldiDriver;
    if (!driver || driver->dldiMagic != DLDI_MAGIC || driver->driverMagic == DLDI_DRIVER_MAGIC_NONE)
    {
        LOG_DEBUG("No dldi driver found\n");
        sIsHandedDownDriver = false;

        // Need to initialize before getting the patch code
        sendToArm9(IPC_COMMAND_ARM9_INITIALIZE_SD_CARD);
        if (!receiveFromArm9())
        {
            LOG_ERROR("Sd card initialization failed\n");
            return false;
        }

        // Try to get the patch code
        sendToArm9(IPC_COMMAND_ARM9_GET_SD_FUNCTIONS);
        if (!receiveFromArm9())
        {
            LOG_ERROR("Getting patch code failed\n");
            return false;
        }

        LOG_DEBUG("Using patch code sd read/write\n");
        ((dldi_header_t*)sDldiBuffer)->readSectorsFuncAddress = (u32)readSectorsWithPatchCode;
        ((dldi_header_t*)sDldiBuffer)->writeSectorsFuncAddress = (u32)writeSectorsWithPatchCode;
    }
    else
    {
        u32 driverSize = 1 << driver->driverSize;
        if (driverSize > sizeof(sDldiBuffer))
        {
            LOG_ERROR("Not enough space for dldi driver of size %d\n", driverSize);
            return false;
        }

        memcpy(sDldiBuffer, driver, driverSize);
        sIsHandedDownDriver = true;

        sDldiDriver.Relocate();
        sDldiDriver.PrepareForUse();

        if (!sDldiDriver.Startup())
        {
            LOG_ERROR("DLDI startup failed\n");
            return false;
        }

        sendToArm9(IPC_COMMAND_ARM9_INITIALIZE_SD_CARD);
        if (!receiveFromArm9())
        {
            LOG_ERROR("Sd card initialization failed\n");
            return false;
        }
    }

    return true;
}

extern "C" bool dldi_readSectors(void* buffer, u32 sector, u32 count)
{
    return sDldiDriver.ReadSectors(sector, count, buffer);
}

extern "C" bool dldi_writeSectors(const void* buffer, u32 sector, u32 count)
{
    return sDldiDriver.WriteSectors(sector, count, buffer);
}

bool dldi_hasDriver()
{
    return sIsHandedDownDriver || sCachedDriver != nullptr;
}

bool dldi_patchTo(dldi_header_t* stub)
{
    if (sCachedDriver)
    {
        return DldiDriver(sCachedDriver).PatchTo(stub);
    }
    return sDldiDriver.PatchTo(stub);
}

void dldi_copyTo(void* target)
{
    // The copy is handed back to Pico Loader when the booted program returns to the
    // launcher, long after this instance is gone, so it must be a complete driver.
    if (sCachedDriver)
    {
        memcpy(target, sCachedDriver, 1u << sCachedDriver->driverSize);
        return;
    }
    memcpy(target, sDldiBuffer, sizeof(sDldiBuffer));
}

/// @brief Checks that a range lies inside the driver's allocated image.
/// @note The range is bounded by the image size rather than by driverEndAddress.
///       driverEndAddress is the end of the driver's initialised data, and a driver's bss
///       legitimately begins there and runs past it, still inside the allocated image.
/// @param driver The driver header.
/// @param imageSize The number of bytes available for the driver image.
/// @param start The first address of the range.
/// @param end One past the last address of the range.
static bool isRangeInsideImage(const dldi_header_t* driver, u32 imageSize, u32 start, u32 end)
{
    return start <= end && driver->driverStartAddress <= start &&
        end - driver->driverStartAddress <= imageSize;
}

static bool isFunctionInsideDriver(const dldi_header_t* driver, u32 function)
{
    return driver->driverStartAddress <= function && function < driver->driverEndAddress;
}

/// @brief Checks that a driver image is a real card driver whose header cannot make
///        DldiDriver::Relocate() or DldiDriver::PrepareForUse() touch memory outside
///        the image. Both do pointer arithmetic and a memset driven purely by header fields.
/// @param driver The driver header.
/// @param imageSize The number of bytes available for the driver image.
static bool isCacheableDriver(const dldi_header_t* driver, u32 imageSize)
{
    if (driver->dldiMagic != DLDI_MAGIC || driver->driverMagic == DLDI_DRIVER_MAGIC_NONE)
    {
        return false;
    }

    // A driver larger than sDldiBuffer also would not fit the space the homebrew
    // bootstub reserves for dldi_copyTo().
    if (driver->driverSize >= 32 ||
        (1u << driver->driverSize) < sizeof(dldi_header_t) ||
        (1u << driver->driverSize) > sizeof(sDldiBuffer) ||
        (1u << driver->driverSize) != imageSize)
    {
        return false;
    }

    if (driver->driverStartAddress >= driver->driverEndAddress ||
        driver->driverEndAddress - driver->driverStartAddress > imageSize)
    {
        return false;
    }

    // A glue, GOT or BSS range is only dereferenced when its fix flag is set, and
    // FIX_ALL replaces the glue and GOT fixes, so unused ranges are not checked.
    if (!(driver->fixFlags & DLDI_FIX_ALL))
    {
        if ((driver->fixFlags & DLDI_FIX_GLUE) &&
            !isRangeInsideImage(driver, imageSize, driver->glueStartAddress, driver->glueEndAddress))
        {
            return false;
        }
        if ((driver->fixFlags & DLDI_FIX_GOT) &&
            !isRangeInsideImage(driver, imageSize, driver->gotStartAddress, driver->gotEndAddress))
        {
            return false;
        }
    }
    if ((driver->fixFlags & DLDI_FIX_BSS) &&
        !isRangeInsideImage(driver, imageSize, driver->bssStartAddress, driver->bssEndAddress))
    {
        return false;
    }

    return isFunctionInsideDriver(driver, driver->startupFuncAddress) &&
        isFunctionInsideDriver(driver, driver->isInsertedFuncAddress) &&
        isFunctionInsideDriver(driver, driver->readSectorsFuncAddress) &&
        isFunctionInsideDriver(driver, driver->writeSectorsFuncAddress) &&
        isFunctionInsideDriver(driver, driver->clearStatusFuncAddress) &&
        isFunctionInsideDriver(driver, driver->shutdownFuncAddress);
}

/// @brief Returns whether two headers describe the same driver build.
///        Only fields that identify the driver are compared. The rest of the image holds
///        the driver's variables, which differ from boot to boot, and the absolute
///        addresses change whenever the driver is relocated, so the layout is compared
///        relative to driverStartAddress. stubSize is excluded because PatchTo() replaces
///        it with the size of whichever stub the driver was patched into.
static bool isSameDriver(const dldi_header_t* a, const dldi_header_t* b)
{
    static constexpr u32 dldi_header_t::* sAddressFields[] =
    {
        &dldi_header_t::driverEndAddress,
        &dldi_header_t::glueStartAddress,
        &dldi_header_t::glueEndAddress,
        &dldi_header_t::gotStartAddress,
        &dldi_header_t::gotEndAddress,
        &dldi_header_t::bssStartAddress,
        &dldi_header_t::bssEndAddress,
        &dldi_header_t::startupFuncAddress,
        &dldi_header_t::isInsertedFuncAddress,
        &dldi_header_t::readSectorsFuncAddress,
        &dldi_header_t::writeSectorsFuncAddress,
        &dldi_header_t::clearStatusFuncAddress,
        &dldi_header_t::shutdownFuncAddress
    };

    if (a->driverMagic != b->driverMagic ||
        a->driverSize != b->driverSize ||
        a->fixFlags != b->fixFlags ||
        a->featureFlags != b->featureFlags ||
        memcmp(a->driverName, b->driverName, sizeof(a->driverName)) != 0)
    {
        return false;
    }

    for (auto field : sAddressFields)
    {
        if (a->*field - a->driverStartAddress != b->*field - b->driverStartAddress)
            return false;
    }

    return true;
}

static void saveDriverCache()
{
    auto driver = (const dldi_header_t*)sDldiBuffer;
    u32 imageSize = 1u << driver->driverSize;
    if (!isCacheableDriver(driver, imageSize))
    {
        LOG_DEBUG("Handed down dldi driver is not cacheable\n");
        return;
    }

    // Skip the write when the saved copy is already this driver.
    auto file = std::make_unique<FIL>();
    if (f_open(file.get(), DLDI_CACHE_PATH, FA_READ) == FR_OK)
    {
        dldi_header_t cachedHeader;
        UINT bytesRead = 0;
        bool isUpToDate = f_read(file.get(), &cachedHeader, sizeof(cachedHeader), &bytesRead) == FR_OK &&
            bytesRead == sizeof(cachedHeader) &&
            f_size(file.get()) == imageSize &&
            isSameDriver(&cachedHeader, driver);
        f_close(file.get());
        if (isUpToDate)
        {
            LOG_DEBUG("Cached dldi driver is up to date\n");
            return;
        }
    }

    // Write to a temporary file first, so an interrupted write can never
    // leave a truncated driver under DLDI_CACHE_PATH.
    LOG_DEBUG("Updating cached dldi driver\n");
    if (f_open(file.get(), DLDI_CACHE_TEMP_PATH, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        LOG_ERROR("Failed to create dldi cache file\n");
        return;
    }
    UINT bytesWritten = 0;
    bool written = f_write(file.get(), driver, imageSize, &bytesWritten) == FR_OK &&
        bytesWritten == imageSize;
    if (f_close(file.get()) != FR_OK || !written)
    {
        // Leave the existing saved copy untouched and discard the partial one.
        LOG_ERROR("Failed to write dldi cache file\n");
        f_unlink(DLDI_CACHE_TEMP_PATH);
        return;
    }

    // Delete the old copy first: f_rename() fails with FR_EXIST instead of
    // replacing a file. A missing old copy is not an error.
    FRESULT result = f_unlink(DLDI_CACHE_PATH);

    // Rename the temporary file to the saved copy's name (DLDI_CACHE_PATH).
    // It is then the saved copy itself, loaded whenever no driver is handed down.
    if ((result != FR_OK && result != FR_NO_FILE) ||
        f_rename(DLDI_CACHE_TEMP_PATH, DLDI_CACHE_PATH) != FR_OK)
    {
        LOG_ERROR("Failed to replace dldi cache file\n");
    }
}

static void loadDriverCache()
{
    auto file = std::make_unique<FIL>();
    if (f_open(file.get(), DLDI_CACHE_PATH, FA_READ) != FR_OK)
    {
        LOG_DEBUG("No cached dldi driver\n");
        return;
    }

    dldi_header_t header;
    UINT bytesRead = 0;
    u32 imageSize = f_size(file.get());
    if (f_read(file.get(), &header, sizeof(header), &bytesRead) != FR_OK ||
        bytesRead != sizeof(header) ||
        !isCacheableDriver(&header, imageSize))
    {
        LOG_ERROR("Cached dldi driver is invalid\n");
        f_close(file.get());
        return;
    }

    // sDldiBuffer cannot hold the cached driver: it holds the patch code entry points
    // that are reading this very file.
    auto image = (dldi_header_t*)memalign(32, imageSize);
    if (!image)
    {
        LOG_ERROR("Not enough memory for cached dldi driver of size %d\n", imageSize);
        f_close(file.get());
        return;
    }

    u32 remaining = imageSize - sizeof(header);
    memcpy(image, &header, sizeof(header));
    if (f_read(file.get(), (u8*)image + sizeof(header), remaining, &bytesRead) != FR_OK ||
        bytesRead != remaining)
    {
        LOG_ERROR("Failed to read cached dldi driver\n");
        free(image);
        f_close(file.get());
        return;
    }
    f_close(file.get());

    // The driver is not relocated or started here. Pico Loader keeps reading the card
    // through the patch code, and DldiDriver::PatchTo() relocates the driver and clears
    // its BSS in the stub of the program being booted, which starts it itself.
    LOG_DEBUG("Using cached dldi driver\n");
    sCachedDriver = image;
}

void dldi_updateDriverCache()
{
    if (sIsHandedDownDriver)
    {
        saveDriverCache();
    }
    else
    {
        loadDriverCache();
    }
}
