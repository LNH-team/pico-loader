#pragma once
#include "patches/Patch.h"
#include "CardiReadRomWithDmaPatchCode.h"

class CardiReadRomWithCpuPatch;

/// @brief Arm9 patch that keeps DMA card ROM reads available on SDK 5 by replacing
///        CARDi_ReadRomWithDma with a CPU read whose completion is delivered from a
///        timer interrupt.
class CardiReadRomWithDmaPatch : public Patch
{
public:
    explicit CardiReadRomWithDmaPatch(const CardiReadRomWithCpuPatch* cardiReadRomWithCpuPatch)
        : _cardiReadRomWithCpuPatch(cardiReadRomWithCpuPatch) { }

    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

    /// @brief Returns whether the patch will be applied. Only valid after FindPatchTarget.
    /// @return True if the patch target was found and all dependencies were resolved.
    constexpr bool WillApply() const { return _cardiReadRomWithDma != nullptr; }

    /// @brief Returns whether the given game needs DMA card ROM reads to stay available.
    /// @param gameCode The game code to check.
    /// @return True if the patch should be used for the game, false otherwise.
    static bool IsNeededForGame(u32 gameCode);

private:
    const CardiReadRomWithCpuPatch* _cardiReadRomWithCpuPatch;
    u32* _cardiReadRomWithDma = nullptr;
    CardiReadRomWithDmaAddresses _addresses;
};
