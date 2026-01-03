#pragma once
#include "../LoaderPlatform.h"
#include "SuperCardCommon.h"
#include "sclite/SuperCardLiteImpl.h"
#include "scsd/SuperCardSDImpl.h"

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
        if (isScLite)
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardReadSectorLitePatchCode(patchHeap, common, changeMode,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSDCommandAndDropLitePatchCode(patchHeap);
                    }),
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardReadDataLitePatchCode(patchHeap);
                    })
                );
            });
        }
        else
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardReadSectorPatchCode(patchHeap, common, changeMode,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSdCommandAndDropPatchCode(patchHeap);
                    }),
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardReadDataPatchCode(patchHeap);
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
        if (isScLite)
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardWriteSectorLitePatchCode(patchHeap, common, changeMode,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSDCommandAndDropLitePatchCode(patchHeap);
                    }),
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardWriteDataLitePatchCode(patchHeap);
                    })
                );
            });
        }
        else
        {
            return patchCodeCollection.GetOrAddSharedPatchCode([&]
            {
                return new SuperCardWriteSectorPatchCode(patchHeap, common, changeMode,
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardSdCommandAndDropPatchCode(patchHeap);
                    }),
                    patchCodeCollection.GetOrAddSharedPatchCode([&]
                    {
                        return new SuperCardWriteDataPatchCode(patchHeap);
                    })
                );
            });
        }
    }

    LoaderPlatformType GetPlatformType() const override { return LoaderPlatformType::Slot2; }

    bool InitializeSdCard() override;

private:
    u16 isScLite = false;

    bool InitializeSdCardIntern();
};
