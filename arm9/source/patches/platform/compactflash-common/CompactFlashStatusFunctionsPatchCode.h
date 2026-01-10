#pragma once
#include "common.h"
#include "sections.h"
#include "../LoaderPlatform.h"
#include "CompactFlashRegisters.h"

DEFINE_SECTION_SYMBOLS(cf_wait_functions);

extern "C" bool cf_waitCardAvailableForCommands();
extern "C" bool cf_waitNextBlockReady();

extern vu16* cf_waitFunctions_reg_cmd;

class CompactFlashStatusFunctionsPatchCode : public PatchCode
{
public:
    CompactFlashStatusFunctionsPatchCode(PatchHeap& patchHeap, const cf_registers_t& registers)
        : PatchCode(SECTION_START(cf_wait_functions), SECTION_SIZE(cf_wait_functions), patchHeap)
    {
        cf_waitFunctions_reg_cmd = registers.command;
    }

    const void* GetWaitCardAvailableForCommandsFunction() const
    {
        return GetAddressAtTarget((void*)cf_waitCardAvailableForCommands);
    }

    const void* GetWaitNextBlockReadyFunction() const
    {
        return GetAddressAtTarget((void*)cf_waitNextBlockReady);
    }
};
