#pragma once
#include "common.h"
#include "sections.h"
#include "patches/PatchCode.h"
#include "thumbInstructions.h"
#include "../IReadSectorsPatchCode.h"
#include "../IWriteSectorsPatchCode.h"
#include "CompactFlashRegisters.h"
#include "ICompactFlashLockUnlockPatchCode.h"

DEFINE_SECTION_SYMBOLS(cf_perform_transfer);
DEFINE_SECTION_SYMBOLS(cf_read_write_functions);

extern "C" bool cf_performTransferSectors(u32 numSectors, u32 sector, u8 command, void* srcAddr, void* dstAddr);

extern "C" bool cf_readSectors(u32 sector, void* buffer, u32 numSectors);
extern "C" bool cf_writeSectors(u32 sector, void* buffer, u32 numSectors);

extern vu16* cf_performTransferSectors_reg_sector_count;
extern u32 cf_performTransferSectors_waitCardAvailableForCommands;
extern u32 cf_performTransferSectors_waitNextBlockReady;

extern vu16* cf_performTransfer_reg_data;
extern u32 cf_performTransfer_performTransferSectors;
extern u32 cf_performTransfer_lockUnlockCard;

extern u16 cf_performTransfer_unlock_label[2];
extern u16 cf_performTransfer_lock_label[2];

class CompactFlashTransferSectorPatchCode : public PatchCode
{
public:
    CompactFlashTransferSectorPatchCode(PatchHeap& patchHeap,
        const cf_registers_t& registers,
        const CompactFlashStatusFunctionsPatchCode* compactFlashStatusFunctionsPatchCode)
        : PatchCode(SECTION_START(cf_perform_transfer), SECTION_SIZE(cf_perform_transfer), patchHeap)
    {
        cf_performTransferSectors_reg_sector_count = registers.sectorCount;
        cf_performTransferSectors_waitCardAvailableForCommands = (u32)compactFlashStatusFunctionsPatchCode->GetWaitCardAvailableForCommandsFunction();
        cf_performTransferSectors_waitNextBlockReady = (u32)compactFlashStatusFunctionsPatchCode->GetWaitNextBlockReadyFunction();
    }

    const void* GetPerformTransferSectorsFunction() const
    {
        return GetAddressAtTarget((void*)cf_performTransferSectors);
    }
};

class CompactFlashReadWriteSectorPatchCode : public PatchCode, public IReadSectorsPatchCode, public IWriteSectorsPatchCode
{
public:
    CompactFlashReadWriteSectorPatchCode(PatchHeap& patchHeap,
        const cf_registers_t& registers,
        const CompactFlashTransferSectorPatchCode* compactFlashTransferSectorPatchCode,
        const ICompactFlashLockUnlockPatchCode* lockUnlockCard)
        : PatchCode(SECTION_START(cf_read_write_functions), SECTION_SIZE(cf_read_write_functions), patchHeap)
    {
        cf_performTransfer_reg_data = registers.data;
    
        cf_performTransfer_performTransferSectors = (u32)compactFlashTransferSectorPatchCode->GetPerformTransferSectorsFunction();
    
        if (lockUnlockCard)
        {
            cf_performTransfer_lockUnlockCard = (u32)lockUnlockCard->GetLockUnlockFunction();
        }
        else
        {
            // what is getting replaced is a `bl`, taking 4 bytes
            const u16 noLockingOpcode = THUMB_MOV_HIREG(THUMB_HI_R8, THUMB_HI_R8);
            cf_performTransfer_unlock_label[0] = noLockingOpcode;
            cf_performTransfer_unlock_label[1] = noLockingOpcode;
            cf_performTransfer_lock_label[0] = noLockingOpcode;
            cf_performTransfer_lock_label[1] = noLockingOpcode;
        }
    }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)cf_readSectors);
    }

    const WriteSectorsFunc GetWriteSectorFunction() const override
    {
        return (const WriteSectorsFunc)GetAddressAtTarget((void*)cf_writeSectors);
    }
};
