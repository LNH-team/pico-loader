#pragma once
#include "common.h"
#include "sections.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"

DEFINE_SECTION_SYMBOLS(cf_read_write_functions);
DEFINE_SECTION_SYMBOLS(cf_read_write_functions_2);

extern "C" bool CF_PerformTransferSectors(u32 numSectors, u32 sector, u8 command, void* srcAddr, void* dstAddr);

extern "C" bool CF_readSectors(u32 sector, void* buffer, u32 numSectors);
extern "C" bool CF_writeSectors(u32 sector, void* buffer, u32 numSectors);

extern u32 cf_readWriteFunctions_reg_sector_count;
extern u32 cf_readWriteFunctions_reg_lba1;
extern u32 cf_readWriteFunctions_reg_lba2;
extern u32 cf_readWriteFunctions_reg_lba3;
extern u32 cf_readWriteFunctions_reg_lba4;
extern u32 cf_readWriteFunctions_reg_command;
extern u32 cf_readWriteFunctions_available_for_command;
extern u32 cf_readWriteFunctions_waitCardNextBlockReady;

extern u32 cf_readWriteFunctions2_reg_data;
extern u32 cf_readWriteFunctions2_performTransferSectors;
extern u32 cf_readWriteFunctions2_lockUnlockCard;

extern u16 CF_PerformTransfer_unlock_label[2];
extern u16 CF_PerformTransfer_lock_label[2];

class CompactFlashTransferSectorPatchCode : public PatchCode
{
public:
    CompactFlashTransferSectorPatchCode(PatchHeap& patchHeap,
        const cf_registers_t& registers,
        const CompactFlashStatusFunctionsPatchCode* compactFlashStatusFunctionsPatchCode)
        : PatchCode(SECTION_START(cf_read_write_functions), SECTION_SIZE(cf_read_write_functions), patchHeap)
    {
        cf_readWriteFunctions_reg_sector_count = registers.sectorCount;
        cf_readWriteFunctions_reg_lba1 = registers.lba1;
        cf_readWriteFunctions_reg_lba2 = registers.lba2;
        cf_readWriteFunctions_reg_lba3 = registers.lba3;
        cf_readWriteFunctions_reg_lba4 = registers.lba4;
        cf_readWriteFunctions_reg_command = registers.command;
        cf_readWriteFunctions_available_for_command = (u32)compactFlashStatusFunctionsPatchCode->GetWaitAvailableForCommandsFunction();
        cf_readWriteFunctions_waitCardNextBlockReady = (u32)compactFlashStatusFunctionsPatchCode->GetWaitCardNextBlockReadyFunction();
    }

    const void* GetPerformTransferSectorsFunction() const
    {
        return GetAddressAtTarget((void*)CF_PerformTransferSectors);
    }
};

class CompactFlashReadSectorPatchCode : public SdReadPatchCode
{
public:
    CompactFlashReadSectorPatchCode(PatchHeap& patchHeap,
        const cf_registers_t& registers,
        const CompactFlashTransferSectorPatchCode* compactFlashTransferSectorPatchCode,
        const CompactFlashLockUnlockPatchCode* lockUnlockCard)
        : SdReadPatchCode(SECTION_START(cf_read_write_functions_2), SECTION_SIZE(cf_read_write_functions_2), patchHeap)
    {
        cf_readWriteFunctions2_reg_data = registers.data;
    
        cf_readWriteFunctions2_performTransferSectors = (u32)compactFlashTransferSectorPatchCode->GetPerformTransferSectorsFunction();
    
        if (lockUnlockCard)
        {
            cf_readWriteFunctions2_lockUnlockCard = (u32)lockUnlockCard->GetLockUnlockFunction();
        }
    }

    const SdReadFunc GetSdReadFunction() const override
    {
        return (const SdReadFunc)GetAddressAtTarget((void*)CF_readSectors);
    }
};

class CompactFlashWriteSectorPatchCode : public SdWritePatchCode
{
public:
    CompactFlashWriteSectorPatchCode(PatchHeap& patchHeap,
        const cf_registers_t& registers,
        const CompactFlashTransferSectorPatchCode* compactFlashTransferSectorPatchCode,
        const CompactFlashLockUnlockPatchCode* lockUnlockCard)
        : SdWritePatchCode(SECTION_START(cf_read_write_functions_2), SECTION_SIZE(cf_read_write_functions_2), patchHeap)
    {
        cf_readWriteFunctions2_reg_data = registers.data;
    
        cf_readWriteFunctions2_performTransferSectors = (u32)compactFlashTransferSectorPatchCode->GetPerformTransferSectorsFunction();
    
        if (lockUnlockCard)
        {
            cf_readWriteFunctions2_lockUnlockCard = (u32)lockUnlockCard->GetLockUnlockFunction();
        }
    }

    const SdWriteFunc GetSdWriteFunction() const override
    {
        return (const SdWriteFunc)GetAddressAtTarget((void*)CF_writeSectors);
    }
};
