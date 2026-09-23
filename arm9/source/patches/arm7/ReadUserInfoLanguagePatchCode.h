#pragma once
#include "sections.h"
#include "ArmHelper.h"
#include "patches/PatchCode.h"

DEFINE_SECTION_SYMBOLS(patch_readuserinfo_language);

extern "C" void readuserinfo_language_copyUserInfo();

extern u32 readuserinfo_language_orrLanguageInstruction;
extern u32 readuserinfo_language_returnInstruction;

/// @brief Patch code replacing the user settings copy in the arm7 ReadUserInfo.
class ReadUserInfoLanguagePatchCode : public PatchCode
{
public:
    /// @brief Constructs the patch code.
    /// @param patchHeap The patch heap to use.
    /// @param language The language to write to bits 0-2 of the user settings language field.
    /// @param returnOffset The offset from the replaced "b next" instruction to "next", at most 0xFF.
    ReadUserInfoLanguagePatchCode(PatchHeap& patchHeap, u32 language, u32 returnOffset)
        : PatchCode(SECTION_START(patch_readuserinfo_language), SECTION_SIZE(patch_readuserinfo_language), patchHeap)
    {
        // The values are written into the instructions themselves, which saves patch space
        readuserinfo_language_orrLanguageInstruction =
            ArmHelper::SetArmImmediate8(readuserinfo_language_orrLanguageInstruction, language & 7);
        readuserinfo_language_returnInstruction =
            ArmHelper::SetArmImmediate8(readuserinfo_language_returnInstruction, returnOffset);
    }

    const void* GetCopyUserInfoFunction() const
    {
        return GetAddressAtTarget((void*)readuserinfo_language_copyUserInfo);
    }
};
