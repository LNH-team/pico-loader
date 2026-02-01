#include "common.h"
#include "ArmHelper.h"
#include "sharedMemory.h"
#include "ndsHeader.h"
#include "moduleParams.h"
#include "patches/platform/LoaderPlatform.h"
#include "Sdk5DsiSdCardRedirectPatchCode.h"
#include "Sdk5DsiSdCardRedirectPatch.h"

static const u32 sAttachFunctionPattern[] = { 0xE92D4018u, 0xE24DDF5Du, 0xE24DDB01u, 0xE59FE050u };
static const u32 sAttachFunctionPatternThumb[] = { 0xB0FFB518u, 0xB0DFB0FFu, 0x4A0E490Du, 0x64CA4469u };

#define BL_TO_GET_DRIVE_STRUCT_OFFSET       (-0x20)
#define BL_TO_GET_DRIVE_STRUCT_OFFSET_ALT   0x88

bool Sdk5DsiSdCardRedirectPatch::FindPatchTarget(PatchContext& patchContext)
{
    _attachFunction = patchContext.FindPattern32Twl(sAttachFunctionPattern, sizeof(sAttachFunctionPattern));
    if (!_attachFunction)
    {
        _attachFunction = patchContext.FindPattern32Twl(sAttachFunctionPatternThumb, sizeof(sAttachFunctionPatternThumb));
        if (_attachFunction)
        {
            _thumb = true;
        }
    }
    if (_attachFunction)
    {
        LOG_DEBUG("Found FATFSi_sdmcRtfsAttach at %p\n", _attachFunction);
        _blToGetDriveStructOffset = BL_TO_GET_DRIVE_STRUCT_OFFSET;
        if (!_thumb && !ArmHelper::IsArmUnconditionalBl(*(u32*)((u8*)_attachFunction + _blToGetDriveStructOffset)))
        {
            _blToGetDriveStructOffset = BL_TO_GET_DRIVE_STRUCT_OFFSET_ALT;
            if (!ArmHelper::IsArmUnconditionalBl(*(u32*)((u8*)_attachFunction + _blToGetDriveStructOffset)))
            {
                LOG_WARNING("Unsupported arm7 version for SD redirection patches\n");
                _attachFunction = nullptr;
            }
        }
    }
    else
    {
        LOG_WARNING("FATFSi_sdmcRtfsAttach not found\n");
    }

    return _attachFunction != nullptr;
}

void Sdk5DsiSdCardRedirectPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_attachFunction)
        return;

    u32 getDriveStructAddress;
    if (_thumb)
    {
        getDriveStructAddress = ArmHelper::GetThumbCallAddress((u32*)((u8*)_attachFunction + _blToGetDriveStructOffset));
    }
    else
    {
        getDriveStructAddress = ArmHelper::GetArmCallAddress((u32*)((u8*)_attachFunction + _blToGetDriveStructOffset));
    }

    auto arm7iAutoload = patchContext.GetAutoloadAdjusterTwl();
    getDriveStructAddress = arm7iAutoload->AdjustInitialToFinal(getDriveStructAddress);

    auto sdRead = patchContext.GetLoaderPlatform()->CreateSdReadPatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
    auto sdWrite = patchContext.GetLoaderPlatform()->CreateSdWritePatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap());
    auto patch = patchContext.GetPatchCodeCollection().AddUniquePatchCode<Sdk5DsiSdCardRedirectPatchCode>
    (
        patchContext.GetPatchHeap(),
        sdRead,
        sdWrite,
        getDriveStructAddress
    );

    if (_thumb)
    {
        *(u32*)((u8*)_attachFunction + 0x44) = (u32)patch->GetIoFunction();
        *(u32*)((u8*)_attachFunction + 0x48) = (u32)patch->GetControlFunction();
    }
    else
    {
        *(u32*)((u8*)_attachFunction + 0x64) = (u32)patch->GetIoFunction();
        *(u32*)((u8*)_attachFunction + 0x68) = (u32)patch->GetControlFunction();
    }
}
