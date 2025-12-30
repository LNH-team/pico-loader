#pragma once
#include "../OverlayPatch.h"

/// @brief Arm9 overlay patch for Kirby Super Star Ultra.
/// @details
/// This game has an issue where it softlocks on the corkboard screen after unlocking new items.
/// This is caused by a race condition where it initializes a save write while it waits for
/// 16 frames for an animation to complete. Normally, the save write finishes after the delay,
/// but if it finishes before, as is common for a flashcart, it will get stuck waiting for
/// the save write which has already completed.
///
/// The code for this is something like:
///     if (SaveCompleteStatus() != 0 || CorkboardNewUnlocksContext->frameCounter < 16) return;
///
/// When it should be in the other order to short circuit properly:
///     if (CorkboardNewUnlocksContext->frameCounter < 16 || SaveCompleteStatus() != 0) return;
///
/// To fix this, the two checks are swapped. This initial assembly:
///     bl     SaveCompleteStatus
///     cmp    r0, #0
///     popne  {r4, pc}
///     ldrb   r0, [r4, #0x50]
///     cmp    r0, #0x10
///     popcc  {r4, pc}
///
/// Is turned into this:
///     ldrb   r0, [r4, #0x50]
///     cmp    r0, #0x10
///     popcc  {r4, pc}
///     bl     SaveCompleteStatus
///     cmp    r0, #0
///     popne  {r4, pc}
///
/// (the offset for the bl also must be adjusted by 3 since it is PC-relative)
class KirbySuperStarUltraPatch : public OverlayPatch
{
public:
    const void* InsertPatch(PatchContext& patchContext) override;
};
