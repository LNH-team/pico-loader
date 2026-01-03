#include "common.h"
#include "patches/PatchContext.h"
#include "thumbInstructions.h"
#include "CardiSetCardDmaPatchCode.h"
#include "patches/platform/LoaderPlatform.h"
#include "patches/arm9/RomOffsetToSdSectorPatchCode.h"
#include "CardiTryReadCardDmaPatch.h"

static const u32 sCARDiTryReadCardDmaPatternPingPals[] = { 0xE92D47F0u, 0xE1A0A000u, 0xE59F9120u, 0xE3A08000u };
static const u32 sCARDiTryReadCardDmaPatternUnknown[] = { 0xE92D47F0u, 0xE59F9138u, 0xE3A06000u, 0xE5998020u };
static const u32 sCARDiTryReadCardDmaPatternUnknown2[] = { 0xE92D47F0u, 0xE59F4134u, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternUnknown3[] = { 0xE92D4FF8u, 0xE59FB140u, 0xE3A05000u, 0xE59B8020u };
static const u32 sCARDiTryReadCardDmaPatternUnknown4[] = { 0xE92D47F0u, 0xE59F9138u, 0xE3A06000u, 0xE599801Cu };
static const u32 sCARDiTryReadCardDmaPattern20029A7[] = { 0xE92D47F0u, 0xE59F9128u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2012774[] = { 0xE92D47F0u, 0xE59F9134u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2017532[] = { 0xE92D47F0u, 0xE59F9130u, 0xE3A07000u, 0xE599401Cu };
static const u32 sCARDiTryReadCardDmaPattern2027530[] = { 0xE92D47F0u, 0xE59F913Cu, 0xE3A06000u, 0xE599801Cu };
static const u32 sCARDiTryReadCardDmaPattern[] = { 0xE92D47F0u, 0xE59F913Cu, 0xE3A06000u, 0xE5998020u };
static const u32 sCARDiTryReadCardDmaPatternSdk3017530[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB14Cu, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3017534[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB148u, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3027530[] = { 0xE92D4FF0u, 0xE24DD004u, 0xE59FB150u, 0xE3A07000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007530[] = { 0xE92D4FF8u, 0xE59FB144u, 0xE3A05000u, 0xE59B8020u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007532[] = { 0xE92D4FF8u, 0xE59F416Cu, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternSdk4027539SpiritTracks[] = { 0xE92D4FF8u, 0xE59F4174u, 0xE3A06000u, 0xE5949020u };
static const u32 sCARDiTryReadCardDmaPatternJpnPokemonDownloader[] = { 0xE92D4FF8u, 0xE59FB138u, 0xE3A05000u, 0xE59B8020u };

static const u32 sCARDiTryReadCardDmaPatternThumbChouSoujuu[] = { 0xB083B5F0u, 0x48399000u, 0x6A0469C5u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk2004F4CThumb[] = { 0xB083B5F0u, 0x4D359000u, 0x94012400u, 0x90020020u };
static const u32 sCARDiTryReadCardDmaPatternSdk2007531Thumb[] = { 0xB083B5F0u, 0x4D379000u, 0x94012400u, 0x90020020u };
static const u32 sCARDiTryReadCardDmaPatternSdk2027530Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A0469C5u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk3007530Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A446A05u, 0x96012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk3012776Thumb[] = { 0xB083B5F0u, 0x483A9000u, 0x6A446A05u, 0x90012600u };
static const u32 sCARDiTryReadCardDmaPatternSdk3017531Thumb[] = { 0xB085B5F0u, 0x483C9000u, 0x6A446A05u, 0x90012000u };
static const u32 sCARDiTryReadCardDmaPatternSdk3027530Thumb[] = { 0xB085B5F0u, 0x483D9000u, 0x6A446A05u, 0x90012000u };
static const u32 sCARDiTryReadCardDmaPatternSdkThumb[] = { 0xB084B5F8u, 0x483D9000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4007531Thumb[] = { 0xB084B5F8u, 0x483E9000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4017530Thumb[] = { 0xB084B5F8u, 0x48479000u, 0x6A446A05u, 0x90032000u };
static const u32 sCARDiTryReadCardDmaPatternSdk4027530Thumb[] = { 0xB084B5F8u, 0x48479000u, 0x6A446A05u, 0x90032000u };

static const u16 sReturnFalsePatchThumb[] = { THUMB_MOVS_IMM(0, 0), THUMB_BX_LR };
static const u32 sReturnFalsePatchArm[] = { 0xE3A00000, 0xE12FFF1E }; // mov r0, #0; bx lr

void CardiTryReadCardDmaPatch::TryPattern(PatchContext& patchContext, const u32* pattern)
{
    _cardiTryReadCardDma = patchContext.FindPattern32(pattern, 16);
    if (_cardiTryReadCardDma)
    {
        _foundPattern = pattern;
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
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternUnknown2);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternUnknown3);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPatternSdk3017534);
        if (!_cardiTryReadCardDma)
           TryPattern(patchContext, sCARDiTryReadCardDmaPattern);
        if (patchContext.GetSdkVersion() < 0x3000000)
        {
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPattern2017532);
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternPingPals);
            if (!_cardiTryReadCardDma)
               TryPattern(patchContext, sCARDiTryReadCardDmaPatternUnknown4);
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

static u32 getArmBlAddress(const u32* instructionPointer)
{
    u32 blInstruction = *instructionPointer;
    return (u32)instructionPointer + 8 + ((int)((blInstruction & 0xFFFFFF) << 8) >> 6);
}

void CardiTryReadCardDmaPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiTryReadCardDma)
        return;

    if (_thumb)
    {
        ((u16*)_cardiTryReadCardDma)[0] = sReturnFalsePatchThumb[0];
        ((u16*)_cardiTryReadCardDma)[1] = sReturnFalsePatchThumb[1];
    }
    else
    {
        bool enableDma = patchContext.GetLoaderPlatform()->HasDmaSdReads();
        if (enableDma)
        {
            u32 cardiCommon;
            u32 cardiOnReadCard;
            u32 cardiSetCardDma;
            u32 miiCardDmaCopy32;
            u32 cardiOnReadCardOffset;
            if (_foundPattern == sCARDiTryReadCardDmaPattern20029A7)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x134);
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x144);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x124));
                if (*(u32*)cardiOnReadCard == 0xE92D40F0u)
                {
                    miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x18));
                    cardiOnReadCardOffset = 0x40;
                }
                else
                {
                    // old version is not supported yet
                    _cardiTryReadCardDma[0] = sReturnFalsePatchArm[0];
                    _cardiTryReadCardDma[1] = sReturnFalsePatchArm[1];
                    return;
                }
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPattern2012774)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x140);
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x150);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x130));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x18));
                cardiOnReadCardOffset = 0x40;
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPattern)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x148) + 4;
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x158);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x138));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x18));
                cardiOnReadCardOffset = 0x40;
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3017530)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x15C) + 4;
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x16C);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x148));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x18));
                cardiOnReadCardOffset = 0x40;
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk3027530)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x160) + 4;
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x170);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x14C));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x18));
                cardiOnReadCardOffset = 0x40;
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4007532)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x178) + 4;
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x188);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x16C));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x1C));
                cardiOnReadCardOffset = 0x48;
            }
            else if (_foundPattern == sCARDiTryReadCardDmaPatternSdk4027539SpiritTracks)
            {
                cardiCommon = *(u32*)((u8*)_cardiTryReadCardDma + 0x180) + 4;
                cardiOnReadCard = *(u32*)((u8*)_cardiTryReadCardDma + 0x190);
                cardiSetCardDma = getArmBlAddress((u32*)((u8*)_cardiTryReadCardDma + 0x174));
                miiCardDmaCopy32 = getArmBlAddress((u32*)(cardiSetCardDma + 0x1C));
                cardiOnReadCardOffset = 0x48;
            }
            else
            {
                _cardiTryReadCardDma[0] = sReturnFalsePatchArm[0];
                _cardiTryReadCardDma[1] = sReturnFalsePatchArm[1];
                return;
            }

            u32 osDisableIrqMask = getArmBlAddress((u32*)(cardiOnReadCard + cardiOnReadCardOffset + 4));

            // patch CARDi_SetCardDma
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
            *(u32*)(cardiSetCardDma + 0) = 0xE51FF004; // ldr pc,= entryAddress
            *(u32*)(cardiSetCardDma + 4) = (u32)cardiSetCardDmaPatchCode->GetCardiSetCardDmaFunction();

            // patch CARDi_OnReadCard
            *(u32*)(cardiOnReadCard + cardiOnReadCardOffset + 0) = 0xE59F0000; // ldr r0,= entryAddress
            *(u32*)(cardiOnReadCard + cardiOnReadCardOffset + 4) = 0xE12FFF30; // blx r0
            *(u32*)(cardiOnReadCard + cardiOnReadCardOffset + 8) = (u32)cardiSetCardDmaPatchCode->GetCardiOnReadCardPatchFunction();

            LOG_DEBUG("DMA enabled\n");
        }
        else
        {
            _cardiTryReadCardDma[0] = sReturnFalsePatchArm[0];
            _cardiTryReadCardDma[1] = sReturnFalsePatchArm[1];
        }
    }
}
