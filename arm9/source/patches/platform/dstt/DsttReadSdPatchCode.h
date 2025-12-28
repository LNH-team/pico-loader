#pragma once
#include "sections.h"
#include "thumbInstructions.h"
#include "patches/PatchCode.h"
#include "../IReadSectorsPatchCode.h"
#include "DsttSdStopTransmissionPatchCode.h"
#include "DsttReadSdApplySectorCommandPatchCode.h"
#include "DsttReadSdWaitDataReadyPatchCode.h"
#include "DsttReadSdTransferDataPatchCode.h"

DEFINE_SECTION_SYMBOLS(dstt_readsd);

extern "C" void dstt_readSd(u32 srcSector, void* dst, u32 sectorCount);

extern u32 dstt_readSd_applySectorCommand_address;
extern u32 dstt_waitDataReady_address;
extern u32 dstt_transferData_address;
extern u32 dstt_stopTransmission_address;

class DsttReadSdPatchCode : public PatchCode, public IReadSectorsPatchCode
{
public:
    explicit DsttReadSdPatchCode(PatchHeap& patchHeap,
        const DsttSdStopTransmissionPatchCode* dsttSdStopTransmissionPatchCode,
        const DsttReadSdApplySectorCommandPatchCode* dsttReadSdApplySectorCommandPatchCode,
        const DsttReadSdWaitDataReadyPatchCode* dsttReadSdWaitDataReadyPatchCode,
        const DsttReadSdTransferDataPatchCode* dsttReadSdTransferDataPatchCode)
        : PatchCode(SECTION_START(dstt_readsd), SECTION_SIZE(dstt_readsd), patchHeap)
        {
            dstt_readSd_applySectorCommand_address = (u32)dsttReadSdApplySectorCommandPatchCode->GetApplySectorCommandFunction();
            dstt_stopTransmission_address = (u32)dsttSdStopTransmissionPatchCode->GetStopTransmissionFunction();
            dstt_waitDataReady_address = (u32)dsttReadSdWaitDataReadyPatchCode->GetWaitDataReadyFunction();
            dstt_transferData_address = (u32)dsttReadSdTransferDataPatchCode->GetTransferDataFunction();
        }

    const ReadSectorsFunc GetReadSectorsFunction() const override
    {
        return (const ReadSectorsFunc)GetAddressAtTarget((void*)dstt_readSd);
    }
};
