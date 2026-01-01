#pragma once
#include "common.h"
#include "sections.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"

DEFINE_SECTION_SYMBOLS(cf_wait_functions);

extern "C" bool cf_waitAvailableForCommands();
extern "C" bool cf_waitCardNextBlockReady();

extern u32 cf_waitFunctions_reg_cmd;
extern u32 cf_waitFunctions_reg_status;

class CompactFlashStatusFunctionsPatchCode : public PatchCode
{
public:
    CompactFlashStatusFunctionsPatchCode(PatchHeap& patchHeap, const CompactFlash::cf_registers_t& registers)
        : PatchCode(SECTION_START(cf_wait_functions), SECTION_SIZE(cf_wait_functions), patchHeap)
    {
        cf_waitFunctions_reg_cmd = registers.command;
        cf_waitFunctions_reg_status = registers.status;
    }

    const void* GetWaitAvailableForCommandsFunction() const
    {
        return GetAddressAtTarget((void*)cf_waitAvailableForCommands);
    }

    const void* GetWaitCardNextBlockReadyFunction() const
    {
        return GetAddressAtTarget((void*)cf_waitCardNextBlockReady);
    }
};
