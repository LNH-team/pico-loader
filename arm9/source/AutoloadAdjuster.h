#pragma once
#include "common.h"
#include "moduleParams.h"

/// @ brief Abstract base class for a general autoload address adjuster.
class IAutoloadAdjuster
{
public:
    /// @brief Adjust an initial address (pre-autoload) to its final location after autoload.
    /// @param addr The address to adjust for autoloading.
    virtual u32 AdjustInitialToFinal(u32 addr) const = 0;

    /// @brief Adjust a final address (post-autoload) to its initial location before autoload.
    /// @param addr The address to adjust for autoloading.
    virtual u32 AdjustFinalToInitial(u32 addr) const = 0;
};

template<typename T>
class AutoloadAdjuster : public IAutoloadAdjuster
{
public:
    AutoloadAdjuster(T* autoloadListStart, T* autoloadListEnd, u32 autoloadStartAddr) :
        _autoloadListStart(autoloadListStart), _autoloadListEnd(autoloadListEnd), _autoloadStartAddr(autoloadStartAddr) { }

    u32 AdjustInitialToFinal(u32 addr) const override;
    u32 AdjustFinalToInitial(u32 addr) const override;

private:
    T* _autoloadListStart;
    T* _autoloadListEnd;
    u32 _autoloadStartAddr;
};

template<typename T>
u32 AutoloadAdjuster<T>::AdjustInitialToFinal(u32 addr) const
{
    u32 currentAddr = _autoloadStartAddr;
    if (!_autoloadListStart || !_autoloadListEnd)
    {
        return addr;
    }
    for (const T* autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
    {
        if (addr >= currentAddr && addr < currentAddr + autoloadListCurr->size)
        {
            addr -= currentAddr;
            addr += autoloadListCurr->targetAddress;
            break;
        }
        else
        {
            currentAddr += autoloadListCurr->size;
        }
    }
    return addr;
}

template<typename T>
u32 AutoloadAdjuster<T>::AdjustFinalToInitial(u32 addr) const
{
    u32 currentAddr = _autoloadStartAddr;
    if (!_autoloadListStart || !_autoloadListEnd)
    {
        return addr;
    }
    for (const T* autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
    {
        if (addr >= autoloadListCurr->targetAddress && addr < autoloadListCurr->targetAddress + autoloadListCurr->size)
        {
            addr -= autoloadListCurr->targetAddress;
            addr += currentAddr;
            break;
        }
        else
        {
            currentAddr += autoloadListCurr->size;
        }
    }
    return addr;
}
