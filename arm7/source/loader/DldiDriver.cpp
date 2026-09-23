#include "common.h"
#include <string.h>
#include "DldiDriver.h"

void DldiDriver::Relocate(u32 targetAddress)
{
    u32 currentAddress = _driver->driverStartAddress;
    if (currentAddress == targetAddress)
        return;

    u32 currentEndAddress = _driver->driverEndAddress;
    
    if (_driver->fixFlags & DLDI_FIX_ALL)
    {
        LOG_WARNING("Dldi driver uses FIX_ALL flag");
        // The header is part of the driver image, and its own address fields are fixed
        // up separately below, so the sweep starts after the header and has to stop at
        // driverEndAddress rather than run for the full length of the image.
        u32* ptr = (u32*)((u8*)_driver + sizeof(dldi_header_t));
        for (u32 address = currentAddress + sizeof(dldi_header_t);
            address < currentEndAddress; address += 4)
        {
            u32 word = *ptr;
            if (currentAddress <= word && word < currentEndAddress)
                *ptr = word - currentAddress + targetAddress;
            ptr++;
        }
    }
    else
    {
        // note that we are not going to do those fixes when we did
        // FIX_ALL already, since it covers them already
        if (_driver->fixFlags & DLDI_FIX_GLUE)
        {
            u32* ptr = (u32*)((u8*)_driver + _driver->glueStartAddress - currentAddress);
            u32 size = _driver->glueEndAddress - _driver->glueStartAddress;
            for (u32 i = 0; i < size; i += 4)
            {
                u32 word = *ptr;
                if (currentAddress <= word && word < currentEndAddress)
                    *ptr = word - currentAddress + targetAddress;
                ptr++;
            }
        }
        if (_driver->fixFlags & DLDI_FIX_GOT)
        {
            u32* ptr = (u32*)((u8*)_driver + _driver->gotStartAddress - currentAddress);
            u32 size = _driver->gotEndAddress - _driver->gotStartAddress;
            for (u32 i = 0; i < size; i += 4)
            {
                u32 word = *ptr;
                if (currentAddress <= word && word < currentEndAddress)
                    *ptr = word - currentAddress + targetAddress;
                ptr++;
            }
        }
    }

    _driver->driverStartAddress = targetAddress;
    _driver->driverEndAddress = _driver->driverEndAddress + targetAddress - currentAddress;
    _driver->glueStartAddress = _driver->glueStartAddress + targetAddress - currentAddress;
    _driver->glueEndAddress = _driver->glueEndAddress + targetAddress - currentAddress;
    _driver->gotStartAddress = _driver->gotStartAddress + targetAddress - currentAddress;
    _driver->gotEndAddress = _driver->gotEndAddress + targetAddress - currentAddress;
    _driver->bssStartAddress = _driver->bssStartAddress + targetAddress - currentAddress;
    _driver->bssEndAddress = _driver->bssEndAddress + targetAddress - currentAddress;

    _driver->startupFuncAddress = _driver->startupFuncAddress + targetAddress - currentAddress;
    _driver->isInsertedFuncAddress = _driver->isInsertedFuncAddress + targetAddress - currentAddress;
    _driver->readSectorsFuncAddress = _driver->readSectorsFuncAddress + targetAddress - currentAddress;
    _driver->writeSectorsFuncAddress = _driver->writeSectorsFuncAddress + targetAddress - currentAddress;
    _driver->clearStatusFuncAddress = _driver->clearStatusFuncAddress + targetAddress - currentAddress;
    _driver->shutdownFuncAddress = _driver->shutdownFuncAddress + targetAddress - currentAddress;
}

void DldiDriver::PrepareForUse()
{
    if (_driver->fixFlags & DLDI_FIX_BSS)
    {
        memset((u8*)_driver + _driver->bssStartAddress - _driver->driverStartAddress, 0, 
            _driver->bssEndAddress - _driver->bssStartAddress);
    }
}

bool DldiDriver::PatchTo(dldi_header_t* stub)
{
    if (_driver->dldiMagic != DLDI_MAGIC)
        return false;

    u32 stubSize = stub->stubSize;
    if (stubSize < _driver->driverSize)
    {
        LOG_ERROR("Dldi stub of size %d is not large enough for driver of size %d\n",
            stubSize, _driver->driverSize);
        return false;
    }

    u32 targetAddress = stub->driverStartAddress;
    u32 driverSize = 1 << _driver->driverSize;

    memcpy(stub, _driver, driverSize);
    stub->stubSize = stubSize;

    auto newDriver = DldiDriver(stub);
    newDriver.Relocate(targetAddress);
    newDriver.PrepareForUse();

    return true;
}
