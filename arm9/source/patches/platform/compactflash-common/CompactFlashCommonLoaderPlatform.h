#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"
#include "CompactFlashLockUnlockPatchCode.h"
#include "CompactFlashStatusFunctions.h"
#include "CompactFlashReadWriteFunctions.h"

/// @brief Base implementation of LoaderPlatform for the Compact Flash slot 2 flashcarts
class CompactFlashCommonLoaderPlatform : public LoaderPlatform
{
public:
    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

protected:
    virtual void CardLockUnlock(bool lock) const = 0;

    virtual bool RequiresLocking() const = 0;
    
    virtual CompactFlashLockUnlockPatchCode* NewCardLockUnlockPatchCode(PatchHeap& patchHeap) const    { return nullptr; }

    virtual const CompactFlash::cf_registers_t& GetCfRegisters() const = 0;

    const SdReadPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        const auto& regs = GetCfRegisters();
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, regs);
        });
        
        auto transferSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashTransferSectorPatchCode(patchHeap, regs, statusFunctions);
        });

        auto lockUnlock = allocateLockUnlockPatchCode(patchCodeCollection, patchHeap);
        
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashReadSectorPatchCode(patchHeap, regs, transferSector, lockUnlock);
        });
    }

    const SdWritePatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        const auto& regs = GetCfRegisters();
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, regs);
        });
        
        auto transferSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashTransferSectorPatchCode(patchHeap, regs, statusFunctions);
        });
        
        auto lockUnlock = allocateLockUnlockPatchCode(patchCodeCollection, patchHeap);
        
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashWriteSectorPatchCode(patchHeap, regs, transferSector, lockUnlock);
        });
    }
    
private:

    const CompactFlashLockUnlockPatchCode* allocateLockUnlockPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const
    {
        if(RequiresLocking())
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return NewCardLockUnlockPatchCode(patchHeap);
            });
        }
        else
        {
            return nullptr;
        }
    }


    bool InitializeCFCard();
};
