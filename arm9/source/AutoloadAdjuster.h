#pragma once
#include "common.h"
#include "moduleParams.h"
#include "IAutoloadAdjuster.h"

/// @brief Class for adjusting addresses for autoload from autoload list entries.
/// @tparam T The type of autoload list entry.
template<typename T>
class AutoloadAdjuster : public IAutoloadAdjuster
{
public:
    AutoloadAdjuster(const T* autoloadListStart, const T* autoloadListEnd, u32 autoloadStartAddress)
        : _autoloadListStart(autoloadListStart), _autoloadListEnd(autoloadListEnd), _autoloadStartAddress(autoloadStartAddress) { }

    u32 AdjustInitialToFinal(u32 initialAddress) const override
    {
        u32 finalAddress = initialAddress;
        u32 currentAddress = _autoloadStartAddress;
        for (auto autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
        {
            if (initialAddress >= currentAddress && initialAddress < currentAddress + autoloadListCurr->size)
            {
                finalAddress -= currentAddress;
                finalAddress += autoloadListCurr->targetAddress;
                break;
            }
            else
            {
                currentAddress += autoloadListCurr->size;
            }
        }
        return finalAddress;
    }

    u32 AdjustFinalToInitial(u32 finalAddress) const override
    {
        u32 initialAddress = finalAddress;
        u32 currentAddress = _autoloadStartAddress;
        for (auto autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
        {
            if (finalAddress >= autoloadListCurr->targetAddress && finalAddress < autoloadListCurr->targetAddress + autoloadListCurr->size)
            {
                initialAddress -= autoloadListCurr->targetAddress;
                initialAddress += currentAddress;
                break;
            }
            else
            {
                currentAddress += autoloadListCurr->size;
            }
        }
        return initialAddress;
    }

private:
    const T* _autoloadListStart;
    const T* _autoloadListEnd;
    u32 _autoloadStartAddress;
};
