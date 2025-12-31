#pragma once
#include "common.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class CompactFlashCommonLoaderPlatform : public LoaderPlatform
{
public:
    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

protected:
	virtual void CardUnlock() const = 0;

	virtual void CardLock() const = 0;

	virtual const CF_REGISTERS& GetCfRegisters() const = 0;

	const SdReadPatchCode* CreateCommonCfReadPatchCode(
		PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap, void* cardLockPatchCode, void* cardUnlockPatchCode) const
	{
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, GetCfRegisters());
        });
	}

	const SdReadPatchCode* CreateCommonCfWritePatchCode(
		PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap, void* cardLockPatchCode, void* cardUnlockPatchCode) const
	{
        auto statusFunctions = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new CompactFlashStatusFunctionsPatchCode(patchHeap, GetCfRegisters());
        });
	}
	
private:
	bool InitializeCFCard();
};
