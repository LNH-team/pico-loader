#include "common.h"
#include "ArmHelper.h"
#include "patches/PatchContext.h"
#include "thumbInstructions.h"
#include "CardiSetCardDmaPatchCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm9/RomOffsetToSdSectorPatchCode.h"
#include "CardiTryReadCardDmaPatch.h"

static const u32 sCARDiTryReadCardDmaPatternPingPals[] = { 0xE92D47F0u, 0xE1A0A000u, 0xE59F9120u, 0xE3A08000u }; // unimplemented
static const u32 sCARDiTryReadCardDmaPatternUnknown[] = { 0xE92D47F0u, 0xE59F9138u, 0xE3A06000u, 0xE5998020u };
static const u32 sCARDiTryReadCardDmaPatternSdk03007532HarvestMoon[] = { 0xE92D47F0u, 0xE59F4134u, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternUnknown3[] = { 0xE92D4FF8u, 0xE59FB140u, 0xE3A05000u, 0xE59B8020u };
static const u32 sCARDiTryReadCardDmaPatternSdk02027533PokemonRanger[] = { 0xE92D47F0u, 0xE59F9138u, 0xE3A06000u, 0xE599801Cu };
static const u32 sCARDiTryReadCardDmaPattern20029A7[] = { 0xE92D47F0u, 0xE59F9128u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2012774[] = { 0xE92D47F0u, 0xE59F9134u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2017532BrainAge[] = { 0xE92D47F0u, 0xE59F9130u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2027530[] = { 0xE92D47F0u, 0xE59F913Cu, 0xE3A06000u, 0xE599801Cu };
static const u32 sCARDiTryReadCardDmaPattern[] = { 0xE92D47F0u, 0xE59F913Cu, 0xE3A06000u, 0xE5998020u };
static const u32 sCARDiTryReadCardDmaPatternSdk3017530[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB14Cu, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3017534[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB148u, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3027530[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB150u, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007530[] = { 0xE92D4FF8u, 0xE59FB144u, 0xE3A05000u, 0xE59B8020u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007532[] = { 0xE92D4FF8u, 0xE59F416Cu, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternSdk4027539SpiritTracks[] = { 0xE92D4FF8u, 0xE59F4174u, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternJpnPokemonDownloader[] = { 0xE92D4FF8u, 0xE59FB138u, 0xE3A05000u, 0xE59B8020u }; // unimplemented

static const u32 sCARDiTryReadCardDmaPatternThumbChouSoujuu[] = { 0xB083B5F0u, 0x48399000u, 0x6A0469C5u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk2004F4CThumb[] = { 0xB083B5F0u, 0x4D359000u, 0x94012400u, 0x90020020u };
static const u32 sCARDiTryReadCardDmaPatternSdk2007531Thumb[] = { 0xB083B5F0u, 0x4D379000u, 0x94012400u, 0x90020020u };
static const u32 sCARDiTryReadCardDmaPatternSdk2027530Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A0469C5u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk3007530Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A446A05u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk3012776Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A446A05u, 0x90012600u }; // TODO does this exist?
static const u32 sCARDiTryReadCardDmaPatternSdk3017531Thumb[] = { 0xB085B5F0u, 0x483C9000u, 0x6A446A05u, 0x90012000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3027530Thumb[] = { 0xB085B5F0u, 0x483D9000u, 0x6A446A05u, 0x90012000u };
static const u32 sCARDiTryReadCardDmaPatternSdkThumb[] = { 0xB084B5F8u, 0x483D9000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007531Thumb[] = { 0xB084B5F8u, 0x483E9000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4017530Thumb[] = { 0xB084B5F8u, 0x48479000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4027530Thumb[] = { 0xB084B5F8u, 0x48479000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4027531ThumbInlined[] = { 0xB083B5F0u, 0x485D9000u, 0x6A062500u, 0x1C286A44u }; // unimplemented

void CardiTryReadCardDmaPatch::TryPattern(PatchContext& patchContext, const u32* pattern)
{
    _cardiTryReadCardDma = patchContext.FindPattern32(pattern, 16);
    if (_cardiTryReadCardDma)
    {
        _foundPattern = pattern;
    }
}

void CardiTryReadCardDmaPatch::ApplyReturnFalsePatch()
{
    if (_thumb)
    {
        ((u16*)_cardiTryReadCardDma)[0] = THUMB_MOVS_IMM(0, 0);
        ((u16*)_cardiTryReadCardDma)[1] = THUMB_BX_LR;
    }
    else
    {
        _cardiTryReadCardDma[0] = 0xE3A00000; // mov r0, #0
        _cardiTryReadCardDma[1] = 0xE12FFF1E; // bx lr
    }
}

bool CardiTryReadCardDmaPatch::FindPatchTarget(PatchContext& patchContext)
{
    if (patchContext.GetSdkVersion() >= 0x4007532)
        TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4007532);
    else if (patchContext.GetSdkVersion() >= 0x4007530)
        TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4007530);
    else if (patchContext.GetSdkVersion() >= 0x3027530)
        TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3027530);
    else if (patchContext.GetSdkVersion() >= 0x3017530)
        TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3017530);
    else if (patchContext.GetSdkVersion() > 0x2027534)
        TryPattern(patchContext, sCARDiTryReadCardDmaPattern);
    else if (patchContext.GetSdkVersion() > 0x2017532)
        TryPattern(patchContext, sCARDiTryReadCardDmaPattern2027530);
    else if (patchContext.GetSdkVersion() > 0x2004FB4)
        TryPattern(patchContext, sCARDiTryReadCardDmaPattern2012774);
    else
        TryPattern(patchContext, sCARDiTryReadCardDmaPattern20029A7);

    if (!_cardiTryReadCardDma)
    {
        if (patchContext.GetSdkVersion() >= 0x4027530)
            TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4027530Thumb);
        else if (patchContext.GetSdkVersion() >= 0x4017530)
            TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4017530Thumb);
        else if (patchContext.GetSdkVersion() >= 0x4007531)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4007531Thumb);
        else if (patchContext.GetSdkVersion() >= 0x3027530)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3027530Thumb);
        else if (patchContext.GetSdkVersion() >= 0x3017531)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3017531Thumb);
        else if (patchContext.GetSdkVersion() >= 0x3017530)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3012776Thumb);
        else if (patchContext.GetSdkVersion() >= 0x3007530)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3007530Thumb);
        else if (patchContext.GetSdkVersion() >= 0x2027530)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk2027530Thumb);
        else if (patchContext.GetSdkVersion() >= 0x2007531)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk2007531Thumb);
        else if (patchContext.GetSdkVersion() >= 0x2004F4C)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk2004F4CThumb);

        if (_cardiTryReadCardDma)
            _thumb = true;
    }

    if (!_cardiTryReadCardDma)
    {
        TryPattern(patchContext, sCARDiTryReadCardDmaPatternUnknown);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4007530);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPattern20029A7);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3017530);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk03007532HarvestMoon);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternUnknown3);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3017534);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPattern);
        if (patchContext.GetSdkVersion() < 0x3000000)
        {
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPattern2017532BrainAge);
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternPingPals);
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk02027533PokemonRanger);
        }

        if (patchContext.GetSdkVersion() >= 0x4000000)
        {
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4027539SpiritTracks);
        }
    }

    if (!_cardiTryReadCardDma)
    {
       TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4007531Thumb);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdkThumb);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk2004F4CThumb);
        if (patchContext.GetSdkVersion() < 0x3000000)
        {
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternThumbChouSoujuu);
        }
        if (!_cardiTryReadCardDma && patchContext.GetSdkVersion() == 0x4027531)
        {
            TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk4027531ThumbInlined);
        }

        if (_cardiTryReadCardDma)
            _thumb = true;
    }

    if (!_cardiTryReadCardDma)
    {
        _cardiTryReadCardDma = patchContext.FindPattern32(sCARDiTryReadCardDmaPatternJpnPokemonDownloader, sizeof(sCARDiTryReadCardDmaPatternJpnPokemonDownloader));
    }

    if (!_cardiTryReadCardDma)
    {
        LOG_WARNING("CARDi_TryReadCardDma not found\n");
    }
    else
    {
        LOG_DEBUG("Found CARDi_TryReadCardDma at %p\n", _cardiTryReadCardDma);
    }

    // There are some sdk roms that do not contain this function at all
    return true; //_cardiTryReadCardDma != nullptr;
}

static u32 resolveCallWithAutoload(u32 callLocation, const IAutoloadAdjuster* autoloadAdjuster, bool isThumb)
{
    s32 callOffset;
    if (isThumb)
    {
        callOffset = ArmHelper::GetThumbCallOffset(((u16*)callLocation)[0], ((u16*)callLocation)[1]);
    }
    else
    {
        callOffset = ArmHelper::GetArmCallOffset(*(u32*)callLocation);
    }
    if (autoloadAdjuster)
    {
        callLocation = autoloadAdjuster->AdjustInitialToFinal(callLocation);
    }
    u32 callDestination = callLocation + callOffset;
    if (isThumb && (callDestination & 3) == 2)
    {
        // fix non-aligned Thumb call to ARM
        callDestination &= ~2u;
    }
    return callDestination;
}

void CardiTryReadCardDmaPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiTryReadCardDma)
        return;

    if (!patchContext.GetLoaderPlatform()->HasDmaSdReads())
    {
        LOG_DEBUG("Platform does not support DMA\n");
        ApplyReturnFalsePatch();
        return;
    }

    u32 cardiCommon;
    u32 cardiOnReadCard;
    u32 cardiSetCardDma;
    u32 cardiSetCardDmaDmaCopyCallOffset;
    u32 cardiOnReadCardDisableIrqCallOffset;
    u32 cardiOnReadCardPatchOffset;
    bool patchUsingPool = false; // needed for some SDK 2-3 Thumb
    u32 cardiOnReadCardPoolOffset;
    if (_thumb)
    {
        if (_foundPattern == sCARDiTryReadCardDmaPatternThumbChouSoujuu)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xEC);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x110) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xE2)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0x94;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk2004F4CThumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xDC);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x100) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xCE)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0x50;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk2007531Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xE4);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x108) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xD6)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0x50;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk2027530Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xF0);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x114) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xE2)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0x9C;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3007530Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xF0) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x114) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xE2)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0xA0;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3017531Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xF8) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x11C) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xEA)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0xA0;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3027530Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xFC) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x120) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xEC)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0xC;
            cardiOnReadCardDisableIrqCallOffset = 0x28;
            cardiOnReadCardPatchOffset = 0x28;
            patchUsingPool = true;
            cardiOnReadCardPoolOffset = 0xA0;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdkThumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0xFC) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x110) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xF2)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0x10;
            cardiOnReadCardDisableIrqCallOffset = 0x32;
            cardiOnReadCardPatchOffset = 0x30;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4007531Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x100) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x114) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0xF4)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0x10;
            cardiOnReadCardDisableIrqCallOffset = 0x32;
            cardiOnReadCardPatchOffset = 0x30;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4017530Thumb || _foundPattern == sCARDiTryReadCardDmaPatternSdk4027530Thumb)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x124) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x138) & ~1u;
            cardiSetCardDma = ArmHelper::GetThumbCallAddress((u16*)((u8*)_cardiTryReadCardDma + 0x118)) & ~1u;
            cardiSetCardDmaDmaCopyCallOffset = 0x10;
            cardiOnReadCardDisableIrqCallOffset = 0x32;
            cardiOnReadCardPatchOffset = 0x30;
        }
        else
        {
            LOG_DEBUG("Unsupported Thumb CARDi_TryReadCardDma signature\n");
            ApplyReturnFalsePatch();
            return;
        }
    }
    else
    {
        if (_foundPattern == sCARDiTryReadCardDmaPatternUnknown)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x144) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x154);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x138));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk03007532HarvestMoon)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x140) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x150);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x134));
            cardiSetCardDmaDmaCopyCallOffset = 0x1C;
            cardiOnReadCardDisableIrqCallOffset = 0x4C;
            cardiOnReadCardPatchOffset = 0x48;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternUnknown3)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x14C) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x15C);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x140));
            cardiSetCardDmaDmaCopyCallOffset = 0x1C;
            cardiOnReadCardDisableIrqCallOffset = 0x4C;
            cardiOnReadCardPatchOffset = 0x48;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk02027533PokemonRanger)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x148);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x158);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x138));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPattern20029A7)
        {
            if (*(u32*)((u8*)_cardiTryReadCardDma + 0x140) == 0xE12FFF1E)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x144);
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x154);
            }
            else
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x134);
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x144);
            }
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x124));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
            if (!ArmHelper::IsArmUnconditionalBl(*(u32*)(cardiSetCardDma + cardiSetCardDmaDmaCopyCallOffset)))
            {
                // some versions have inlined MIi_CardDmaCopy32
                LOG_DEBUG("Failed to patch sCARDiTryReadCardDmaPattern20029A7\n");
                ApplyReturnFalsePatch();
                return;
            }
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPattern2012774)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x140);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x150);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x130));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPattern2017532BrainAge)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x13C);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x14C);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x130));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPattern2027530)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x148);
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x158);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x138));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPattern)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x148) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x158);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x138));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3017530)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x15C) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x16C);
            if (*(u32*)((u8*)_cardiTryReadCardDma + 0x158) == 0xE12FFF1E)
            {
                cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x148));
            }
            else
            {
                cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x14C));
            }
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3017534)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x158) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x168);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x148));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3027530)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x160) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x170);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x14C));
            cardiSetCardDmaDmaCopyCallOffset = 0x18;
            cardiOnReadCardDisableIrqCallOffset = 0x44;
            cardiOnReadCardPatchOffset = 0x40;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4007530)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x150) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x160);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x144));
            cardiSetCardDmaDmaCopyCallOffset = 0x1C;
            cardiOnReadCardDisableIrqCallOffset = 0x4C;
            cardiOnReadCardPatchOffset = 0x48;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4007532)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x178) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x188);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x16C));
            cardiSetCardDmaDmaCopyCallOffset = 0x1C;
            cardiOnReadCardDisableIrqCallOffset = 0x4C;
            cardiOnReadCardPatchOffset = 0x48;
        }
        else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4027539SpiritTracks)
        {
            cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x180) + 4;
            cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x190);
            cardiSetCardDma = ArmHelper::GetArmCallAddress((u32*)((u8*)_cardiTryReadCardDma + 0x174));
            cardiSetCardDmaDmaCopyCallOffset = 0x1C;
            cardiOnReadCardDisableIrqCallOffset = 0x4C;
            cardiOnReadCardPatchOffset = 0x48;
        }
        else
        {
            LOG_DEBUG("Unsupported ARM CARDi_TryReadCardDma signature\n");
            ApplyReturnFalsePatch();
            return;
        }
    }

    // correct all addresses for autoload, if libcard is in an autoload block (New Super Mario Bros.)
    auto autoloadAdjuster = patchContext.GetAutoloadAdjuster();

    // CARDi_OnReadCard address is the final location, but we need its initial location to patch it now
    if (autoloadAdjuster)
    {
        cardiOnReadCard = autoloadAdjuster->AdjustFinalToInitial(cardiOnReadCard);
    }

    // MIi_CardDmaCopy32 and OS_DisableIrqMask are relative, but we need their final locations to call them later
    u32 miiCardDmaCopy32 = resolveCallWithAutoload(cardiSetCardDma + cardiSetCardDmaDmaCopyCallOffset, autoloadAdjuster, _thumb);
    u32 osDisableIrqMask = resolveCallWithAutoload(cardiOnReadCard + cardiOnReadCardDisableIrqCallOffset, autoloadAdjuster, _thumb);

    // begin with patching
    auto sdReadDmaPatchCode = patchContext.GetLoaderPlatform()->CreateSdReadDmaPatchCode(
        patchContext.GetPatchCodeCollection(), patchContext.GetPatchHeap(), (const void*)miiCardDmaCopy32);
    auto romOffsetToSdSectorPatchCode = patchContext.GetPatchCodeCollection().GetOrAddSharedPatchCode([&]
    {
        return new RomOffsetToSdSectorPatchCode(patchContext.GetPatchHeap(),
            (const rom_file_info_t*)((u32)SHARED_ROM_FILE_INFO - 0x02F00000 + 0x02700000));
    });
    auto cardiSetCardDmaPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<CardiSetCardDmaPatchCode>
    (
        patchContext.GetPatchHeap(),
        romOffsetToSdSectorPatchCode,
        sdReadDmaPatchCode,
        (const void*)cardiCommon,
        (const void*)osDisableIrqMask
    );

    if (_thumb)
    {
        // patch CARDi_SetCardDma
        *(u16*)(cardiSetCardDma + 0) = 0x4800; // ldr r0,= entryAddress
        *(u16*)(cardiSetCardDma + 2) = 0x4700; // bx r0
        *(u32*)(cardiSetCardDma + 4) = (u32)cardiSetCardDmaPatchCode->GetCardiSetCardDmaFunction();

        // patch CARDi_OnReadCard
        if (patchUsingPool)
        {
            *(u16*)(cardiOnReadCard + cardiOnReadCardPatchOffset) = 0x4780; // blx r0 (pre-loaded from pool)
            *(u32*)(cardiOnReadCard + cardiOnReadCardPoolOffset) = (u32)cardiSetCardDmaPatchCode->GetCardiOnReadCardPatchFunction();
        }
        else
        {
            *(u16*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 0) = 0x4800; // ldr r0,= entryAddress
            *(u16*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 2) = 0x4780; // blx r0
            *(u32*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 4) = (u32)cardiSetCardDmaPatchCode->GetCardiOnReadCardPatchFunction();
            *(u16*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 8) = 0x4600; // nop
        }
    }
    else
    {
        // patch CARDi_SetCardDma
        *(u32*)(cardiSetCardDma + 0) = 0xE51FF004; // ldr pc,= entryAddress
        *(u32*)(cardiSetCardDma + 4) = (u32)cardiSetCardDmaPatchCode->GetCardiSetCardDmaFunction();

        // patch CARDi_OnReadCard
        *(u32*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 0) = 0xE59F0000; // ldr r0,= entryAddress
        *(u32*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 4) = 0xE12FFF30; // blx r0
        *(u32*)(cardiOnReadCard + cardiOnReadCardPatchOffset + 8) = (u32)cardiSetCardDmaPatchCode->GetCardiOnReadCardPatchFunction();
    }

    LOG_DEBUG("DMA enabled\n");
}
