#pragma once
#include "../Patch.h"

/// @brief Arm9 patch for intercepting calls to SVC_GetCRC16 in The Last Window: The Secret of Cape West
/// @details
/// This game has DS Protect 1.27, which is patched by the regular DS Protect patches, but also has
/// an extra layer of CRC checks that must match or the game will fail to boot.
///
/// It first runs DS Protect NotA1 at startup, and then does a CRC of most of DS Protect. This can
/// tell which DS Protect functions were run, because at decrypt-run-encrypt, the key gets changed.
/// So the CRC is sensitive not only to code modifications, but also to which encrypted functions
/// were run, and how many times they were run.
///
/// The current DS Protect patch modifies NotA1, causes one of its subfunction to run twice, and
/// the other subfunction to not be run, which obviously breaks the CRC.
///
/// Another region is also CRCed, from 0202DB34 to 02030334. What resides here is unknown, it does not
/// contain any commonly modified functions such as AP or card reading.
///
/// To make things more annoying, the memory location of the correct CRC is randomized, and the code
/// that checks the CRC resides in a compressed overlay which is loaded into ITCM. This same function
/// also receives several dummy checks that are expected to fail, and so cannot be patched to always
/// report the CRC matches.
///
/// Instead, the function call that calculates the CRC is shimmed and if its arguments match
/// a problematic CRC calculation, the expected answer is returned instead.
class LastWindowCrcPatch : public Patch
{
public:
    bool FindPatchTarget(PatchContext& patchContext) override;
    void ApplyPatch(PatchContext& patchContext) override;

private:
    u32* _getCrc16 = nullptr;

    u32 MakeBlxCall(u32 patchAddr) const
    {
        return 0xFA000000 | (((patchAddr - (u32)_getCrc16 - 8) >> 2) & 0xFFFFFF);
    }
};
