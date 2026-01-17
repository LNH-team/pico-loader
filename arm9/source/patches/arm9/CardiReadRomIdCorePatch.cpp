#include "common.h"
#include "thumbInstructions.h"
#include "../PatchContext.h"
#include "../FunctionSignature.h"
#include "CardiReadRomIdCorePatchCode.h"
#include "CardiReadRomIdCorePatch.h"

static constexpr auto sSignaturesArm = std::to_array<const FunctionSignature>
({
    { (const u32[]) { 0xE92D4000u, 0xE24DD004u, 0xE3A0032Eu, 0xE3A01000u }, 0x02004F50, 0x04017531 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFF0Bu }, 0x03007532, 0x03017531 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFF08u }, 0x03017530, 0x03017534 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFF07u }, 0x03027530, 0x04007531 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFEF2u }, 0x04007531, 0x04007531 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFEFDu }, 0x04007532, 0x04007532 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFEFBu }, 0x04017530, 0x04027539 },
    { (const u32[]) { 0xE92D4008u, 0xE3A0032Eu, 0xE3A01000u, 0xEBFFFEE6u }, 0x04017533, 0x04017533 },
    { (const u32[]) { 0xE92D4010u, 0xE3A04000u, 0xE1A01004u, 0xE3A0032Eu }, 0x04027537, 0x04027539 }
});

static constexpr auto sSignaturesThumb = std::to_array<const FunctionSignature>
({
    { (const u32[]) { 0xB081B500u, 0x2100480Bu, 0xF958F000u, 0x6801480Au }, 0x02017530, 0x02017530 },
    { (const u32[]) { 0xB081B500u, 0x2100480Bu, 0xF98AF000u, 0x6801480Au }, 0x03007530, 0x03012776 },
    { (const u32[]) { 0xB081B500u, 0x2100480Bu, 0xF990F000u, 0x6801480Au }, 0x03007532, 0x03007532 },
    { (const u32[]) { 0x202EB508u, 0x21000680u, 0xFE8CF7FFu, 0x68014809u }, 0x03017531, 0x03017531 },
    { (const u32[]) { 0xB081B500u, 0x2100480Bu, 0xF98EF000u, 0x6801480Au }, 0x03017531, 0x03017534 },
    { (const u32[]) { 0xB081B500u, 0x2100480Cu, 0xF992F000u, 0x6800480Bu }, 0x03027530, 0x03027532 },
    { (const u32[]) { 0x202EB508u, 0x21000680u, 0xFE8AF7FFu, 0x6800480Au }, 0x03027530, 0x04007531 },
    { (const u32[]) { 0x202EB508u, 0x21000680u, 0xFE74F7FFu, 0x6800480Au }, 0x04017530, 0x04027539 },
    { (const u32[]) { 0x202EB508u, 0x21000680u, 0xFE62F7FFu, 0x6800480Au }, 0x04027531, 0x04027531 },
    { (const u32[]) { 0x202EB508u, 0x21000680u, 0xFE56F7FFu, 0x6800480Au }, 0x04027537, 0x04027537 }
});

static constexpr auto sSignaturesArmSdk5 = std::to_array<const FunctionSignature>
({
    { (const u32[]) { 0xE92D4010u, 0xE3A040B8u, 0xEBFFFFD6u, 0xE3500000u }, 0x0500753B, 0x05027534 },
    { (const u32[]) { 0xE92D4008u, 0xE3A000B8u, 0xE3A01000u, 0xEBFFFFCEu }, 0x05017536, 0x05047531 },
    { (const u32[]) { 0xE92D4038u, 0xE3A050B8u, 0xEBFFFFD7u, 0xE3500000u }, 0x05017537, 0x05057535 },
    { (const u32[]) { 0xE92D4010u, 0xE3A04000u, 0xE1A01004u, 0xE3A000B8u }, 0x05037531, 0x05057535 }
});

static constexpr auto sSignaturesThumbSdk5 = std::to_array<const FunctionSignature>
({
    { (const u32[]) { 0x24B8B510u, 0xFFC4F7FFu, 0xD0002800u, 0x1C202490u }, 0x05007538, 0x05017537 },
    { (const u32[]) { 0x20B8B508u, 0xF7FF2100u, 0x480AFFABu, 0x480A6801u }, 0x05017537, 0x05057535 },
    { (const u32[]) { 0x24B8B510u, 0xFFC6F7FFu, 0xD0002800u, 0x1C202490u }, 0x05037531, 0x05057534 }
});

bool CardiReadRomIdCorePatch::FindPatchTarget(PatchContext& patchContext)
{
    if (patchContext.GetSdkVersion().IsTwlSdk())
    {
        for (const auto& signature : sSignaturesArmSdk5)
        {
            if (CheckSignature(patchContext, signature))
            {
                break;
            }
        }

        if (!_cardiReadRomIdCore)
        {
            for (const auto& signature : sSignaturesThumbSdk5)
            {
                if (CheckSignature(patchContext, signature))
                {
                    _thumb = true;
                    break;
                }
            }
        }
    }
    else
    {
        for (const auto& signature : sSignaturesArm)
        {
            if (CheckSignature(patchContext, signature))
            {
                break;
            }
        }

        if (!_cardiReadRomIdCore)
        {
            for (const auto& signature : sSignaturesThumb)
            {
                if (CheckSignature(patchContext, signature))
                {
                    _thumb = true;
                    break;
                }
            }
        }
    }

    if (!_cardiReadRomIdCore)
    {
        LOG_WARNING("CARDi_ReadRomIDCore not found\n");
    }

    return true; //_cardiReadRomIdCore != nullptr;
}

void CardiReadRomIdCorePatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiReadRomIdCore)
        return;

    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<CardiReadRomIdCorePatchCode>
    (
        patchContext.GetPatchHeap(),
        (const void*)(patchContext.GetSdkVersion().IsTwlSdk() ? 0x02FFFC00 : 0x027FFC00)
    );

    if (_thumb)
    {
        ((u16*)_cardiReadRomIdCore)[0] = 0x4800;
        ((u16*)_cardiReadRomIdCore)[1] = 0x4700;
    }
    else
    {
        _cardiReadRomIdCore[0] = 0xE51FF004;
    }

    _cardiReadRomIdCore[1] = (u32)patchCode->GetCardiReadRomIdCoreFunction();
}

bool CardiReadRomIdCorePatch::CheckSignature(const PatchContext& patchContext, const FunctionSignature& signature)
{
    if (patchContext.GetSdkVersion() >= signature.GetMinimumSdkVersion() &&
        patchContext.GetSdkVersion() <= signature.GetMaximumSdkVersion())
    {
        _cardiReadRomIdCore = patchContext.FindPattern32(signature.GetPattern(), 16);
        if (_cardiReadRomIdCore)
        {
            LOG_DEBUG("Found CARDi_ReadRomIDCore at %p\n", _cardiReadRomIdCore);
            return true;
        }
    }

    return false;
}
