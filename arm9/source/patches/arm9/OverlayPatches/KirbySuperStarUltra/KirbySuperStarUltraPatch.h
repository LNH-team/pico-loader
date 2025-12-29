#pragma once
#include "../OverlayPatch.h"

/// @brief Arm9 overlay patch for Kirby Super Star Ultra.
class KirbySuperStarUltraPatch : public OverlayPatch
{
public:
    KirbySuperStarUltraPatch() { }

    const void* InsertPatch(PatchContext& patchContext) override;
};
