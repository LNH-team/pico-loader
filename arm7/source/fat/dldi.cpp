#include "common.h"
#include <string.h>
#include "ipc.h"
#include "ipcCommands.h"
#include "loader/DldiDriver.h"

static u8 sDldiBuffer[16 * 1024] alignas(32);
static DldiDriver sDldiDriver = DldiDriver((dldi_header_t*)sDldiBuffer);

bool dldi_init()
{
    auto driver = (const dldi_header_t*)gLoaderHeader.dldiDriver;
    if (!driver || driver->dldiMagic != DLDI_MAGIC || driver->driverMagic == DLDI_DRIVER_MAGIC_NONE)
    {
        LOG_DEBUG("No dldi driver found\n");

        // Need to initialize before getting the patch code
        sendToArm9(IPC_COMMAND_ARM9_INITIALIZE_SD_CARD);
        if (!receiveFromArm9())
        {
            LOG_ERROR("Sd card initialization failed\n");
            return false;
        }

        // Ask the ARM9 to build a complete, valid DLDI driver at 0x037F8000
        // (see handleGetSdFunctionsCommand() in arm9/source/main.cpp).
        //
        // It must be a real driver, not just entry points: this buffer serves
        // two purposes. We call through it ourselves, exactly as the
        // handed-down-driver branch above does, and we later patch it into
        // booted homebrew with DldiDriver::PatchTo(). PatchTo() validates
        // dldiMagic and refuses anything without it, so a pair of bare
        // function pointers would leave booted homebrew running its own
        // unpatched placeholder stub, whose reads always fail.
        sendToArm9(IPC_COMMAND_ARM9_GET_SD_FUNCTIONS);
        if (!receiveFromArm9())
        {
            LOG_ERROR("Getting patch code failed\n");
            return false;
        }

        LOG_DEBUG("Using patch code sd driver\n");
        memcpy(sDldiBuffer, (const void*)0x037F8000, sizeof(sDldiBuffer));

        // The driver's function-pointer fields, and the adapters' internal
        // jump-target words, were written for the 0x037F8000 staging address.
        // It lives in sDldiBuffer now, so without relocating, every call
        // through it would jump back into 0x037F8000 - correct only for as
        // long as that shared staging address happens to still hold these
        // bytes. Same sequence as the handed-down-driver branch below.
        sDldiDriver.Relocate();
        sDldiDriver.PrepareForUse();

        if (!sDldiDriver.Startup())
        {
            LOG_ERROR("DLDI startup failed\n");
            return false;
        }
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

bool dldi_patchTo(dldi_header_t* stub)
{
    return sDldiDriver.PatchTo(stub);
}

void dldi_copyTo(void* target)
{
    memcpy(target, sDldiBuffer, sizeof(sDldiBuffer));
}
