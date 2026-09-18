#pragma once
#include "patches/Patch.h"

class CardiReadRomWithDmaPatch;

/// @brief Arm9 patch to disable DMA card reads on SDK 5.
class CardiIsRomDmaAvailablePatch : public Patch
{
public:
    /// @param cardiReadRomWithDmaPatch Optional patch that keeps DMA reads available.
    ///        When it is applied, DMA reads are left enabled.
    explicit CardiIsRomDmaAvailablePatch(const CardiReadRomWithDmaPatch* cardiReadRomWithDmaPatch = nullptr)
        : _cardiReadRomWithDmaPatch(cardiReadRomWithDmaPatch) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    const CardiReadRomWithDmaPatch* _cardiReadRomWithDmaPatch;
    u32* _cardiIsRomDmaAvailable = nullptr;
    u16 _thumb = false;
};
