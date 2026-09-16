#pragma once
#include "patches/PatchCode.h"
#include "sections.h"

DEFINE_SECTION_SYMBOLS(patch_cardireadromwithdma);
DEFINE_SECTION_SYMBOLS(patch_cardireadromwithdma_irq);

extern "C" void patch_cardireadromwithdma_entry(void);
extern "C" void patch_cardireadromwithdma_timer_irq(void);

extern u32 patch_cardireadromwithdma_in_flight_slot;
extern u32 patch_cardireadromwithdma_read_rom_with_cpu;
extern u32 patch_cardireadromwithdma_timer_irq_address;
extern u32 patch_cardireadromwithdma_os_set_irq_function;
extern u32 patch_cardireadromwithdma_os_reset_request_irq_mask;
extern u32 patch_cardireadromwithdma_os_enable_irq_mask;

extern u32 patch_cardireadromwithdma_irq_in_flight_slot;
extern u32 patch_cardireadromwithdma_irq_state;
extern u32 patch_cardireadromwithdma_irq_large_frame;
extern u32 patch_cardireadromwithdma_irq_completion_tail;

/// @brief Addresses in the game that the SDK 5 card DMA read replacement depends on.
struct CardiReadRomWithDmaAddresses
{
    /// @brief Card DMA state structure.
    u32 state;
    /// @brief Slot in the card DMA state structure holding the request in flight.
    u32 inFlightSlot;
    /// @brief Non-zero if the card DMA interrupt handler saves r4-r6, zero if it saves only r4.
    /// @note This is not a bool: byte-sized fields of patch objects, which are allocated in
    ///       the loader heap in LCDC VRAM, are not retained under melonDS.
    u32 largeFrame;
    /// @brief Completion tail of the card DMA interrupt handler.
    u32 completionTail;
    /// @brief CARDi_ReadRomWithCPU. +1 if Thumb.
    u32 readRomWithCpu;
    /// @brief OS_SetIrqFunction.
    u32 osSetIrqFunction;
    /// @brief OS_ResetRequestIrqMask.
    u32 osResetRequestIrqMask;
    /// @brief OS_EnableIrqMask.
    u32 osEnableIrqMask;
};

/// @brief Timer interrupt handler that delivers the completion of a card DMA read.
class CardiReadRomWithDmaIrqPatchCode : public PatchCode
{
public:
    CardiReadRomWithDmaIrqPatchCode(PatchHeap& patchHeap, const CardiReadRomWithDmaAddresses& addresses)
        : PatchCode(SECTION_START(patch_cardireadromwithdma_irq), SECTION_SIZE(patch_cardireadromwithdma_irq), patchHeap)
    {
        patch_cardireadromwithdma_irq_in_flight_slot = addresses.inFlightSlot;
        patch_cardireadromwithdma_irq_state = addresses.state;
        patch_cardireadromwithdma_irq_large_frame = addresses.largeFrame;
        patch_cardireadromwithdma_irq_completion_tail = addresses.completionTail;
    }

    const void* GetTimerIrqFunction() const
    {
        return GetAddressAtTarget((void*)patch_cardireadromwithdma_timer_irq);
    }
};

/// @brief Replacement for CARDi_ReadRomWithDma.
class CardiReadRomWithDmaPatchCode : public PatchCode
{
public:
    CardiReadRomWithDmaPatchCode(PatchHeap& patchHeap, const CardiReadRomWithDmaAddresses& addresses,
        const CardiReadRomWithDmaIrqPatchCode* irqPatchCode)
        : PatchCode(SECTION_START(patch_cardireadromwithdma), SECTION_SIZE(patch_cardireadromwithdma), patchHeap)
    {
        patch_cardireadromwithdma_in_flight_slot = addresses.inFlightSlot;
        patch_cardireadromwithdma_read_rom_with_cpu = addresses.readRomWithCpu;
        patch_cardireadromwithdma_timer_irq_address = (u32)irqPatchCode->GetTimerIrqFunction();
        patch_cardireadromwithdma_os_set_irq_function = addresses.osSetIrqFunction;
        patch_cardireadromwithdma_os_reset_request_irq_mask = addresses.osResetRequestIrqMask;
        patch_cardireadromwithdma_os_enable_irq_mask = addresses.osEnableIrqMask;
    }

    const void* GetCardiReadRomWithDmaFunction() const
    {
        return GetAddressAtTarget((void*)patch_cardireadromwithdma_entry);
    }
};
