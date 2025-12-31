#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"
#include "CompactFlashStatusFunctions.h"
#include "CompactFlashReadWriteFunctions.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class CompactFlashCommonLoaderPlatform : public LoaderPlatform
{
public:
    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

protected:
	virtual void CardUnlock() const = 0;

	virtual void CardLock() const = 0;

	virtual const CompactFlash::CF_REGISTERS& GetCfRegisters() const = 0;

	const SdReadPatchCode* CreateCommonCfReadPatchCode(
		PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap, const void* cardLockUnlockPatchCode) const
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
		
		return patchCodeCollection.GetOrAddSharedPatchCode([&]
		{
			return new CompactFlashReadSectorPatchCode(patchHeap, regs, transferSector, cardLockUnlockPatchCode);
		});
	}

	const SdWritePatchCode* CreateCommonCfWritePatchCode(
		PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap, const void* cardLockUnlockPatchCode) const
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
		
		return patchCodeCollection.GetOrAddSharedPatchCode([&]
		{
			return new CompactFlashWriteSectorPatchCode(patchHeap, regs, transferSector, cardLockUnlockPatchCode);
		});
	}
	
private:
	bool InitializeCFCard();
};
