#include "common.h"
#include "../../../PatchContext.h"
#include "thumbInstructions.h"
#include "gameCode.h"
#include "DSProtectOverlayPatchAsm.h"
#include "DSProtectOverlayPatch.h"

#define FUNCMASK_A1     (0b000001)
#define FUNCMASK_NOTA1  (0b000010)
#define FUNCMASK_A2     (0b000100)
#define FUNCMASK_NOTA2  (0b001000)
#define FUNCMASK_A3     (0b010000)
#define FUNCMASK_NOTA3  (0b100000)

#define PATCH_WRITE_WORD  (0)
#define PATCH_COPY_NEXT   (1)

#define MAGIC_100_102    (0xFFFFFE70)
#define MAGIC_105_123    (0xFFFFFCE0)
#define MAGIC_200s_205s  (0x00000000)

#define OFFSET_INVALID  (0x80000000)

const void* DSProtectOverlayPatch::InsertPatch(PatchContext& patchContext)
{
    ConfigurePatch(patchContext);

    u32 patchSize = SECTION_SIZE(dsprotectpatch);
    void* patchAddress = patchContext.GetPatchHeap().Alloc(patchSize);
    u32 entryAddress = (u32)&dsprotectpatch_entry - (u32)SECTION_START(dsprotectpatch) + (u32)patchAddress;
    memcpy(patchAddress, SECTION_START(dsprotectpatch), patchSize);

    return (const void*)entryAddress;
}

void DSProtectOverlayPatch::ConfigurePatch(PatchContext& patchContext) const
{
    // Next patch and target overlay ID
    dsprotectpatch_nextAddress = next ? (const void*)next->InsertPatch(patchContext) : nullptr;
    dsprotectpatch_overlayId = _overlayId;

    u32 regionOffset = _overlayOffset;

    // Default invalid, enable below
    dsprotectpatch_offsetA1 = OFFSET_INVALID;
    dsprotectpatch_offsetNotA1 = OFFSET_INVALID;

    switch (_version)
    {
        case DSProtectVersion::v1_00_2:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_100_102;

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xD8;
                regionOffset += 0xDC; // __DSprot_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xD8;
                regionOffset += 0xDC; // __DSprot_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_05:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0x24; // setCacheDisabled
            regionOffset += 0xB4; // Encryptor_StartRange
            regionOffset += 0xD4; // Encryptor_EndRange
            regionOffset += 0x74; // RC4_Init
            regionOffset += 0x4C; // RC4_Byte
            regionOffset += 0x24; // RC4_InitSBox
            regionOffset += 0x9C; // RC4_EncryptInstructions
            regionOffset += 0xC;  // RC4_DecryptInstructions
            regionOffset += 0x58; // RC4_InitAndEncryptInstructions
            regionOffset += 0x58; // RC4_InitAndDecryptInstructions

            if (_functionMask & (FUNCMASK_A2 | FUNCMASK_NOTA2))
            {
                regionOffset += 0x120; // MACOwner_IsBad
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x19C; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
                regionOffset += 0x110; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xD8;
                regionOffset += 0xDC; // __DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xD8;
                regionOffset += 0xDC; // __DSprot_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_06:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0x24; // setCacheDisabled
            regionOffset += 0xB4; // Encryptor_StartRange
            regionOffset += 0xD4; // Encryptor_EndRange
            regionOffset += 0x74; // RC4_Init
            regionOffset += 0x4C; // RC4_Byte
            regionOffset += 0x24; // RC4_InitSBox
            regionOffset += 0x9C; // RC4_EncryptInstructions
            regionOffset += 0xC;  // RC4_DecryptInstructions
            regionOffset += 0x58; // RC4_InitAndEncryptInstructions
            regionOffset += 0x58; // RC4_InitAndDecryptInstructions

            if (_functionMask & (FUNCMASK_A2 | FUNCMASK_NOTA2))
            {
                regionOffset += 0x120; // MACOwner_IsBad
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x1A4; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
                regionOffset += 0x110; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xD4;
                regionOffset += 0xD8; // DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xD4;
                regionOffset += 0xD8; // DSProt_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_08:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0x24; // setCacheDisabled
            regionOffset += 0xB4; // Encryptor_StartRange
            regionOffset += 0xD4; // Encryptor_EndRange
            regionOffset += 0x74; // RC4_Init
            regionOffset += 0x4C; // RC4_Byte
            regionOffset += 0x24; // RC4_InitSBox
            regionOffset += 0x9C; // RC4_EncryptInstructions
            regionOffset += 0xC;  // RC4_DecryptInstructions
            regionOffset += 0x58; // RC4_InitAndEncryptInstructions
            regionOffset += 0x58; // RC4_InitAndDecryptInstructions

            if (_functionMask & (FUNCMASK_A2 | FUNCMASK_NOTA2))
            {
                regionOffset += 0x120; // MACOwner_IsBad
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x19C; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
                regionOffset += 0x110; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xD4;
                regionOffset += 0xD8; // DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xD4;
                regionOffset += 0xD8; // DSProt_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_10:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0xCC; // Encryptor_StartRange
            regionOffset += 0xE8; // Encryptor_EndRange
            regionOffset += 0x74; // RC4_Init
            regionOffset += 0x4C; // RC4_Byte
            regionOffset += 0x24; // RC4_InitSBox
            regionOffset += 0x9C; // RC4_EncryptInstructions
            regionOffset += 0xC;  // RC4_DecryptInstructions
            regionOffset += 0x58; // RC4_InitAndEncryptInstructions
            regionOffset += 0x58; // RC4_InitAndDecryptInstructions

            if (_functionMask & (FUNCMASK_A2 | FUNCMASK_NOTA2))
            {
                regionOffset += 0x120; // MACOwner_IsBad
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x19C; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
                regionOffset += 0x110; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xD8;
                regionOffset += 0xE0; // DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xD8;
                regionOffset += 0xE0; // DSProt_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_20:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0xC8; // Encryptor_StartRange
            regionOffset += 0xE4; // Encryptor_EndRange

            if (_functionMask & FUNCMASK_A2)
            {
                regionOffset += 0x118; // MACOwner_IsBad
            }

            if (_functionMask & FUNCMASK_NOTA2)
            {
                regionOffset += 0x118; // MACOwner_IsGood
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x1D4; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
            }

            if (_functionMask & FUNCMASK_A1)
            {
                regionOffset += 0x108; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                regionOffset += 0x108; // ROMTest_IsGood
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xE0;
                regionOffset += 0xE8; // DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xE0;
                regionOffset += 0xE8; // DSProt_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_22:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            regionOffset += 0xC8; // Encryptor_StartRange
            regionOffset += 0xE4; // Encryptor_EndRange

            if (_functionMask & FUNCMASK_A2)
            {
                regionOffset += 0x120; // MACOwner_IsBad
            }

            if (_functionMask & FUNCMASK_NOTA2)
            {
                regionOffset += 0x120; // MACOwner_IsGood
            }

            if (_functionMask & (FUNCMASK_A1 | FUNCMASK_NOTA1))
            {
                regionOffset += 0x1D4; // ROMUtil_Read
                regionOffset += 0xA4;  // ROMUtil_CRC32
            }

            if (_functionMask & FUNCMASK_A1)
            {
                regionOffset += 0x108; // ROMTest_IsBad
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                regionOffset += 0x108; // ROMTest_IsGood
            }

            if (_functionMask & FUNCMASK_A1)
            {
                dsprotectpatch_offsetA1 = regionOffset + 0xE0;
                regionOffset += 0xE8; // DSProt_DetectFlashcart
            }

            if (_functionMask & FUNCMASK_NOTA1)
            {
                dsprotectpatch_offsetNotA1 = regionOffset + 0xE0;
                regionOffset += 0xE8; // DSProt_DetectNotFlashcart
            }

            break;
        }
        case DSProtectVersion::v1_23:
        case DSProtectVersion::v1_23Z:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_105_123;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0xAC;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xB8 + 0xAC;

            break;
        }
        case DSProtectVersion::v1_25:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0x90;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xA4 +0x9C;

            break;
        }
        case DSProtectVersion::v1_26:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0x90;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xA4 + 0x9C;

            break;
        }
        case DSProtectVersion::v1_27:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0x8C;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xA0 + 0x8C;

            break;
        }
        case DSProtectVersion::v1_28:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0x8C;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xA0 + 0x8C;

            break;
        }
        case DSProtectVersion::v2_00:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0x0 + 0x188;
            dsprotectpatch_offsetNotA1 = regionOffset + 0x19C + 0x188;

            break;
        }
        case DSProtectVersion::v2_01:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0xAB0 + 0x188;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xC4C + 0x188;

            break;
        }
        case DSProtectVersion::v2_03:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0xC1C + 0x18C;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xDBC + 0x18C;

            break;
        }
        case DSProtectVersion::v2_05:
        {
            dsprotectpatch_patchType = PATCH_COPY_NEXT;

            dsprotectpatch_offsetA1 = regionOffset + 0xCAC + 0x18C;
            dsprotectpatch_offsetNotA1 = regionOffset + 0xE4C + 0x18C;

            break;
        }
        // Instant versions
        case DSProtectVersion::v2_00s:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_200s_205s;

            dsprotectpatch_offsetA1 = regionOffset + 0x94 + 0x118;

            break;
        }
        case DSProtectVersion::v2_01s:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_200s_205s;

            dsprotectpatch_offsetA1 = regionOffset + 0x94 + 0x118;

            break;
        }
        case DSProtectVersion::v2_03s:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_200s_205s;

            dsprotectpatch_offsetA1 = regionOffset + 0xE0 + 0x118;

            break;
        }
        case DSProtectVersion::v2_05s:
        {
            dsprotectpatch_patchType = PATCH_WRITE_WORD;
            dsprotectpatch_writeWord = MAGIC_200s_205s;

            dsprotectpatch_offsetA1 = regionOffset + 0x104 + 0x118;

            break;
        }
        default:
        {
            LOG_WARNING("Unsupported DSProtect version\n");
            break;
        }
    }
}
