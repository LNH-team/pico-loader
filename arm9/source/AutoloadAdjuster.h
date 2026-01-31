#pragma once
#include "common.h"
#include "moduleParams.h"
#include "IAutoloadAdjuster.h"

template<typename T>
class AutoloadAdjuster : public IAutoloadAdjuster
{
public:
    AutoloadAdjuster(const T* autoloadListStart, const T* autoloadListEnd, u32 autoloadStartAddress) :
        _autoloadListStart(autoloadListStart), _autoloadListEnd(autoloadListEnd), _autoloadStartAddress(autoloadStartAddress) { }

    u32 AdjustInitialToFinal(u32 initialAddress) const override
    {
        u32 currentAddress = _autoloadStartAddress;
        for (auto autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
        {
            if (initialAddress >= currentAddress && initialAddress < currentAddress + autoloadListCurr->size)
            {
                initialAddress -= currentAddress;
                initialAddress += autoloadListCurr->targetAddress;
                break;
            }
            else
            {
                currentAddress += autoloadListCurr->size;
            }
        }
        return initialAddress;
    }

    u32 AdjustFinalToInitial(u32 finalAddress) const override
    {
        u32 currentAddress = _autoloadStartAddress;
        for (auto autoloadListCurr = _autoloadListStart; autoloadListCurr != _autoloadListEnd; autoloadListCurr++)
        {
            if (finalAddress >= autoloadListCurr->targetAddress && finalAddress < autoloadListCurr->targetAddress + autoloadListCurr->size)
            {
                finalAddress -= autoloadListCurr->targetAddress;
                finalAddress += currentAddress;
                break;
            }
            else
            {
                currentAddress += autoloadListCurr->size;
            }
        }
        return finalAddress;
    }

private:
    const T* _autoloadListStart;
    const T* _autoloadListEnd;
    u32 _autoloadStartAddress;
};
