#pragma once
#include "common.h"
#include "sections.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"

DEFINE_SECTION_SYMBOLS(cf_read_write_functions);

extern "C" bool cf_waitAvailableForCommands();
extern "C" bool cf_waitCardNextBlockReady();

extern u32 cf_waitFunctions_reg_cmd;
extern u32 cf_waitFunctions_reg_status;

class CompactFlashStatusFunctionsPatchCode : public PatchCode
{
public:
    CompactFlashStatusFunctionsPatchCode(PatchHeap& patchHeap, const CF_REGISTERS& registers)
        : SdReadPatchCode(SECTION_START(scsd_read_sector), SECTION_SIZE(scsd_read_sector), patchHeap)
    {
		cf_waitFunctions_reg_cmd = registers.command;
		cf_waitFunctions_reg_status = registers.status;
    }

    const void* GetWaitAvailableForCommands() const override
    {
        return GetAddressAtTarget((void*)cf_waitAvailableForCommands);
    }

    const void* GetWaitCardNextBlockReady() const override
    {
        return GetAddressAtTarget((void*)cf_waitCardNextBlockReady);
    }
};
