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

    const IReadSectorsPatchCode* CreateSdReadPatchCode(
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
            return new CompactFlashReadWriteSectorPatchCode(patchHeap, regs, transferSector, lockUnlock);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
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
            return new CompactFlashReadWriteSectorPatchCode(patchHeap, regs, transferSector, lockUnlock);
        });
    }

protected:
    /// @brief Locks/Unlocks the cart to operate on the inserted CF card
    /// @param Whether the card sould be locked (prevent R/W operations) or unlocked (allows R/W operations)
    virtual void CardLockUnlock(bool lock) const = 0;

    /// @brief Checks if the implementation requires specific lock/unlock commands before using the CF Card
    ///        If a card requires locking, it should implement \see NewCardLockUnlockPatchCode returning the patch
    ///        code performing the lock/unlock sequences, and also repeat the same logic in SetCardLocked.
    /// @return \c true if the platform requires the sequence, or \c false otherwise.
    virtual bool RequiresLocking() const = 0;

    /// @brief Generates the patch code containing the lock/unlock routines equivalent to \see SetCardLocked
    /// @note  The returned routine, is only allowed to modify r0, other registers should be left untouched
    /// @return A pointer to the allocated \see CompactFlashLockUnlockPatchCode
    virtual CompactFlashLockUnlockPatchCode* NewCardLockUnlockPatchCode(PatchHeap& patchHeap) const { return nullptr; }

    /// @brief Returns the exposed address associated to the Compact Flash registers
    /// @return the \see cf_registers_t struct containing the registers
    virtual const cf_registers_t& GetCfRegisters() const = 0;

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


    bool InitializeCfCard();
};
