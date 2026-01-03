#pragma once
#include "../OverlayPatch.h"

/// @brief Arm9 overlay patch for Rabbids Go Home.
/// @details
/// This game has a homebrew AP method based on ROM read timings. Specifically, it will
/// run 100 ROM reads of 0x4000 bytes each, or 32 0x200-byte pages, and use ticks to time how
/// long it takes. It must take between 0x80000 and 0x88000 ticks, or 1.001 to 1.063 seconds,
/// or the game will hang prior to the title screen (after the publisher splash and language selection).
/// The actual contents of the ROM reads are not checked, and they are obtained using TWL SDK CARD_ReadRom.
///
/// To bypass this, most of the function that runs the AP check is skipped, as it has no other side effects.
class RabbidsGoHomePatch : public OverlayPatch
{
public:
    const void* InsertPatch(PatchContext& patchContext) override;
};
