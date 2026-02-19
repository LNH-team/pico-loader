#pragma once
#include "../LoaderPlatform.h"
#include "SuperCardCommon.h"
#include "sclite/SuperCardLiteReadSectorPatchCode.h"
#include "sclite/SuperCardLiteSendSdCommandPatchCode.h"
#include "sclite/SuperCardLiteWriteSectorPatchCode.h"
#include "scsd/SuperCardSDReadSectorPatchCode.h"
#include "scsd/SuperCardSDSendSdCommandPatchCode.h"
#include "scsd/SuperCardSDWriteSectorPatchCode.h"

/// @brief Implementation of LoaderPlatform for the slot 2 SuperCard flashcard
class SuperCardLoaderPlatform : public LoaderPlatform
{
public:
    const IReadSectorsPatchCode* CreateSdReadPatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto common = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new SuperCardCommonPatchCode(patchHeap);
        });
        auto changeMode = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new SuperCardChangeModePatchCode(patchHeap);
        });
		auto sendSdCommand = CreateSdCommandPatchCode(patchCodeCollection, patchHeap);
        if (isScLite || isSuperChis)
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardReadSectorLitePatchCode(patchHeap, common, changeMode,
                    sendSdCommand,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardLiteReadDataPatchCode(patchHeap);
                    })
                );
            });
        }
        else
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardSDReadSectorPatchCode(patchHeap, common, changeMode,
                    sendSdCommand,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSDReadDataPatchCode(patchHeap);
                    })
                );
            });
        }
    }

    const IWriteSectorsPatchCode* CreateSdWritePatchCode(
        PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const override
    {
        auto common = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new SuperCardCommonPatchCode(patchHeap);
        });
        auto changeMode = patchCodeCollection.GetOrAddSharedPatchCode([&]
        {
            return new SuperCardChangeModePatchCode(patchHeap);
        });
		auto sendSdCommand = CreateSdCommandPatchCode(patchCodeCollection, patchHeap);
        if (isScLite)
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardLiteWriteSectorPatchCode(patchHeap, common, changeMode,
                    sendSdCommand,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardLiteWriteDataPatchCode(patchHeap);
                    })
                );
            });
        }
        else
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardSDWriteSectorPatchCode(patchHeap, common, changeMode,
                    sendSdCommand,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSDWriteDataPatchCode(patchHeap);
                    })
                );
            });
        }
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

private:
	const ISuperCardSendSdCommandPatchCode* CreateSdCommandPatchCode(
	    PatchCodeCollection& patchCodeCollection, PatchHeap& patchHeap) const
	{
		if (isScLite)
		{
			return patchCodeCollection.GetOrAddSharedPatchCode([&]
			{
				return new SuperCardLiteSendSdCommandPatchCode(patchHeap);
			});
		}
		else
		{
			return patchCodeCollection.GetOrAddSharedPatchCode([&]
			{
				return new SuperCardSDSendSdCommandPatchCode(patchHeap);
			});
		}
	}

    u16 isScLite = false;
    u16 isSuperChis = false;

    bool InitializeSdCardIntern();
};
