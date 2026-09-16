#include "common.h"
#include "ArmHelper.h"
#include "gameCode.h"
#include "patches/PatchContext.h"
#include "CardiReadRomWithCpuPatch.h"
#include "CardiReadRomWithDmaPatch.h"

namespace
{
    struct CardDmaHandlerSignature
    {
        u32 pattern[4];
        /// @brief Non-zero if the handler saves r4-r6, zero if it saves only r4.
        u32 largeFrame;
    };
}

// The card library of SDK 5 games is part of the static arm9 module, so the
// addresses found here are already the addresses the code runs at.
//
// The card DMA transfer completion handler is searched for instead of
// CARDi_ReadRomWithDma, because its first instructions contain no relative
// branches. CARDi_ReadRomWithDma directly follows the handler.
static constexpr CardDmaHandlerSignature sCardDmaHandlerSignatures[] =
{
    { { 0xE92D4070u, 0xE59F508Cu, 0xE5954008u, 0xE3540000u }, 1 }, // request in flight at state + 8
    { { 0xE92D4070u, 0xE59F508Cu, 0xE595400Cu, 0xE3540000u }, 1 }, // request in flight at state + 0xC
    { { 0xE92D4010u, 0xE59F008Cu, 0xE5904008u, 0xE3540000u }, 0 }, // state reloaded from the literal pool
};

static constexpr u32 sCardDataPortAddress = 0x04100010;
static constexpr u32 sReadRomWithDmaOffset = 0xA0;
static constexpr u32 sReadRomWithDmaSearchSize = 0x80;
static constexpr u32 sCompletionTailOffset = 0x48;
static constexpr u32 sBranchToCompletionTailOffset = 0x38;
static constexpr u32 sBranchToCompletionTail = 0x0A000002; // beq completion tail
static constexpr u32 sReadRomWithDmaCallCount = 5;

bool CardiReadRomWithDmaPatch::IsNeededForGame(u32 gameCode)
{
    // These games start an asynchronous ROM read and then wait for its completion
    // callback without letting the card thread run, so a read that is queued for
    // the card thread instead of performed with DMA never completes.
    // Other games keep DMA reads disabled: the replacement performs the whole read
    // before returning, which would stall a caller that expects to keep running
    // while an asynchronous read is in progress.
    // The replacement uses timer 3, which none of these games use.
    switch (gameCode)
    {
        case GAMECODE("BD7E"): // Digging for Dinosaurs
        case GAMECODE("B7FE"): // The Magic School Bus: Oceans
        case GAMECODE("TKSE"): // 2 Game Pack: My Amusement Park + Digging for Dinosaurs
        case GAMECODE("TGSE"): // I Spy Game Pack
        case GAMECODE("BIUE"): // I Spy Universe
        case GAMECODE("BA2E"): // Animal Planet: Vet Collection
            return true;
        default:
            return false;
    }
}

bool CardiReadRomWithDmaPatch::FindPatchTarget(PatchContext& patchContext)
{
    const u32* readRomWithCpu = _cardiReadRomWithCpuPatch->GetCardiReadRomWithCpu();
    if (!readRomWithCpu)
    {
        LOG_WARNING("CARDi_ReadRomWithDma not patched, CARDi_ReadRomWithCPU not found\n");
        return true;
    }

    for (const auto& signature : sCardDmaHandlerSignatures)
    {
        u32* handler = patchContext.FindPattern32(signature.pattern, sizeof(signature.pattern));
        if (!handler)
        {
            continue;
        }

        u32* readRomWithDma = handler + sReadRomWithDmaOffset / 4;
        if (handler[sBranchToCompletionTailOffset / 4] != sBranchToCompletionTail ||
            (readRomWithDma[0] & 0xFFFF4000) != 0xE92D4000) // push { ..., lr }
        {
            LOG_WARNING("Unexpected card DMA handler layout at %p\n", handler);
            return true;
        }

        u32 handlerAddress = (u32)handler;
        // handler[1] is ldr rX, [pc, #imm], loading the card DMA state structure.
        u32 state = handler[(4 + 8 + (handler[1] & 0xFFF)) / 4];

        bool poolHasHandler = false;
        bool poolHasDataPort = false;
        u32 calls[sReadRomWithDmaCallCount];
        u32 callCount = 0;
        for (u32 i = 0; i < sReadRomWithDmaSearchSize / 4; i++)
        {
            u32 word = readRomWithDma[i];
            poolHasHandler |= word == handlerAddress;
            poolHasDataPort |= word == sCardDataPortAddress;
            if (callCount < sReadRomWithDmaCallCount && ArmHelper::IsArmUnconditionalBl(word))
            {
                calls[callCount++] = (u32)&readRomWithDma[i] + ArmHelper::GetArmCallOffset(word);
            }
        }

        // The calls are OS_DisableInterrupts, OS_SetIrqFunction, OS_ResetRequestIrqMask,
        // OS_EnableIrqMask and OS_RestoreInterrupts, in that order.
        if (!poolHasHandler || !poolHasDataPort || callCount != sReadRomWithDmaCallCount)
        {
            LOG_WARNING("Unexpected CARDi_ReadRomWithDma layout at %p\n", readRomWithDma);
            return true;
        }

        _addresses.state = state;
        // handler[2] is ldr r4, [rX, #imm], loading the request in flight.
        _addresses.inFlightSlot = state + (handler[2] & 0xFFF);
        _addresses.largeFrame = signature.largeFrame;
        _addresses.completionTail = handlerAddress + sCompletionTailOffset;
        _addresses.readRomWithCpu = (u32)readRomWithCpu | (_cardiReadRomWithCpuPatch->IsThumb() ? 1 : 0);
        _addresses.osSetIrqFunction = calls[1];
        _addresses.osResetRequestIrqMask = calls[2];
        _addresses.osEnableIrqMask = calls[3];
        _cardiReadRomWithDma = readRomWithDma;

        LOG_DEBUG("Found CARDi_ReadRomWithDma at %p\n", _cardiReadRomWithDma);
        return true;
    }

    LOG_WARNING("CARDi_ReadRomWithDma not found\n");
    return true;
}

void CardiReadRomWithDmaPatch::ApplyPatch(PatchContext& patchContext)
{
    if (!_cardiReadRomWithDma)
        return;

    auto irqPatchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<CardiReadRomWithDmaIrqPatchCode>(
        patchContext.GetPatchHeap(), _addresses);
    auto patchCode = patchContext.GetPatchCodeCollection().AddUniquePatchCode<CardiReadRomWithDmaPatchCode>(
        patchContext.GetPatchHeap(), _addresses, irqPatchCode);

    _cardiReadRomWithDma[0] = 0xE51FF004; // ldr pc, [pc, #-4]
    _cardiReadRomWithDma[1] = (u32)patchCode->GetCardiReadRomWithDmaFunction();
}
