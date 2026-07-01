#pragma once
#include "../LoaderPlatform.h"
#include "Ez5hReadSdSectorPatchCode.h"
#include "Ez5hReadWriteSdPatchCode.h"
#include "Ez5hSendCommandPatchCode.h"
#include "Ez5hWriteSdSectorPatchCode.h"

/// @brief Implementation of LoaderPlatform for the DATEL line of flashcarts
class Ez5hLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
		return static_cast<const IReadSectorsPatchCode*>(CreateSdReadWritePatchCode(patchCodeCollection, patchHeap));
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
		return static_cast<const IWriteSectorsPatchCode*>(CreateSdReadWritePatchCode(patchCodeCollection, patchHeap));
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot1; }

    bool InitializeSdCard() override;
private:
	const Ez5hReadWriteSdPatchCode* CreateSdReadWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const
	{
        auto sdioCrc = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSdioCrcPatchCode(patchHeap);
        });
        auto sendWriteDataCommand = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSendWriteDataRomPatchCode(patchHeap);
        });
        auto sendCommand = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hSendCommandPatchCode(patchHeap);
        });
        auto writeSdSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hWriteSdSectorPatchCode(patchHeap);
        });
        auto readSdSector = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hReadSdSectorPatchCode(patchHeap);
        });
        return patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new Ez5hReadWriteSdPatchCode(patchHeap, sdioCrc, sendWriteDataCommand, sendCommand, writeSdSector, readSdSector);
        });
	}
};
