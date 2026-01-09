#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"
#include "CompactFlashStatusFunctionsPatchCode.h"
#include "CompactFlashReadWriteSectorPatchCode.h"
#include "ICompactFlashLockUnlockPatchCode.h"

/// @brief Base implementation of LoaderPlatform for the Compact Flash slot 2 flashcarts
class CompactFlashCommonLoaderPlatform : public LoaderPlatform
{
public:
    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        const auto& registers = GetCfRegisters();
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, registers);
        });

        auto transferSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashTransferSectorPatchCode(patchHeap, registers, statusFunctions);
        });

        auto lockUnlock = CreateLockingPatchCode(patchCodeCollection, patchHeap);

        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashReadWriteSectorPatchCode(patchHeap, registers, transferSector, lockUnlock);
        });
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        const auto& registers = GetCfRegisters();
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, registers);
        });

        auto transferSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashTransferSectorPatchCode(patchHeap, registers, statusFunctions);
        });

        auto lockUnlock = CreateLockingPatchCode(patchCodeCollection, patchHeap);

        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashReadWriteSectorPatchCode(patchHeap, registers, transferSector, lockUnlock);
        });
    }

protected:
    /// @brief Locks/Unlocks the cart to operate on the inserted CF card
    /// @param locked Whether the card should be locked (prevent R/W operations) or unlocked (allows R/W operations)
    virtual void SetCardLocked(bool locked) const = 0;

    /// @brief Generates the patch code containing the lock/unlock routines equivalent to \see SetCardLocked
    ///        If a card requires no locking, this doesn't have to be implemented.
    /// @note  The returned routine, is only allowed to modify r0, other registers should be left untouched
    /// @return A pointer to the allocated \see ICompactFlashLockUnlockPatchCode
    virtual const ICompactFlashLockUnlockPatchCode* CreateLockingPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const { return nullptr; }

    /// @brief Returns the exposed address associated to the Compact Flash registers
    /// @return the \see cf_registers_t struct containing the registers
    virtual const cf_registers_t& GetCfRegisters() const = 0;

private:
    bool InitializeCfCard();
};
