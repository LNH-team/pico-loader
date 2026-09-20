#pragma once
#include "sections.h"
#include "Ez5hSendCommandPatchCode.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(ez5h_do_sd_operation);

extern u32 ez5h_doSDOperation_sendSDIOCommand;

extern "C" int ez5h_doSDOperation(uint32_t sector, void* buffer, uint32_t num_sectors, bool(*operation)(u32 sector, void* buffer));

class Ez5hDoSdOperationPatchCode : public PatchCode
{
public:
    Ez5hDoSdOperationPatchCode(PatchHeap& patchHeap,
        const Ez5hSendCommandPatchCode* ez5hSendCommandPatchCode)
        : PatchCode(SECTION_START(ez5h_do_sd_operation), SECTION_SIZE(ez5h_do_sd_operation), patchHeap)
    {
        ez5h_doSDOperation_sendSDIOCommand = (u32)ez5hSendCommandPatchCode->GetSendSDIOCommandFunction();
    }

    const void* GetDoSDOperationFunction() const
    {
        return GetAddressAtTarget((void*)ez5h_doSDOperation);
    }
};
