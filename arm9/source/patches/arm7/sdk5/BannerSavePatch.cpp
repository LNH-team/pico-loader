#include "common.h"
#include "thumbInstructions.h"
#include "BannerSavePatch.h"

// patch target is inside FATFSi_ResolveIPLPath
static const u32 sPatchTarget1[] = { 0xE0405004u, 0xE28D0066u, 0xE1A01004u, 0xE2852001u };
static const u32 sPatchTarget2[] = { 0xE0407005u, 0xE28D9066u, 0xE1A00009u, 0xE1A01005u };
static const u32 sPatchTarget3[] = { 0xE0405004u, 0xE28D0066u, 0xE1A01004u, 0xE1A02005u };

static const u32 sPatchTargetThumb1[] = { 0x1BC0D070u, 0xAC22900Cu, 0x34029A0Cu, 0x1C391C20u };

bool BannerSavePatch::FindPatchTarget(PatchContext& patchContext)
{
    _patchLocation = patchContext.FindPattern32Twl(sPatchTarget1, sizeof(sPatchTarget1));
    if (_patchLocation != nullptr)
    {
        _foundSignature = sPatchTarget1;
    }
    else
    {
        _patchLocation = patchContext.FindPattern32Twl(sPatchTarget2, sizeof(sPatchTarget2));
        if (_patchLocation != nullptr)
        {
            _foundSignature = sPatchTarget2;
        }
        else
        {
            _patchLocation = patchContext.FindPattern32Twl(sPatchTarget3, sizeof(sPatchTarget3));
            if (_patchLocation != nullptr)
            {
                _foundSignature = sPatchTarget3;
            }
            else
            {
                _patchLocation = patchContext.FindPattern32Twl(sPatchTargetThumb1, sizeof(sPatchTargetThumb1));
                if (_patchLocation != nullptr)
                {
                    _foundSignature = sPatchTargetThumb1;
                }
            }
        }
    }

    if (_patchLocation != nullptr)
    {
        LOG_DEBUG("Banner save patch: %p\n", _patchLocation);
    }

    return _patchLocation != nullptr;
}

void BannerSavePatch::ApplyPatch(PatchContext& patchContext)
{
    if (_patchLocation == nullptr)
    {
        return;
    }

    if (_foundSignature == sPatchTarget1 || _foundSignature == sPatchTarget3)
    {
        // ldr     r1,=Lxx_#0x2FA1C5C
        // mov     r0,r9
        // bl      Lxx_#0x2F9E9C0
        // cmp     r0,#0x0
        // bne     Lxx_#0x2F9E5E0
        // bl      Lxx_#0x37C3BE4
        // cmp     r0,#0x3
        // bne     Lxx_#0x2F9E5E0
        // ldr     r1,=Lxx_#0x2FA1C7C                -> nop
        // mov     r0,r4                             -> nop
        // bl      Lxx_#0x37C4844                    -> nop
        // cmp     r0,#0x0                           -> nop
        // beq     Lxx_#0x2F9E718    <- PatchTarget  -> nop
        // sub     r5,r0,r4                          -> nop
        // add     r0,r13,#0x66                      -> nop
        // mov     r1,r4                             -> nop
        // add     r2,r5,#0x1                        -> nop
        // bl      Lxx_#0x2F9DE08                    -> nop
        // ldr     r2,=Lxx_#0x103                    -> nop
        // add     r0,r13,#0x66
        // ldr     r1,=Lxx_#0x2FA1C88                -> change the pool address to the banner save path
        // add     r0,r0,r5,lsl #0x1                 -> nop
        // sub     r2,r2,r5                          -> mov r2, #64
        // bl      Lxx_#0x2F9DE08
        // add     r9,r13,#0x66
        // mov     r6,#0x6
        // b       Lxx_#0x2F9E718
        _patchLocation[-5] = 0;
        _patchLocation[-4] = 0;
        _patchLocation[-3] = 0;
        _patchLocation[-2] = 0;
        _patchLocation[-1] = 0;
        _patchLocation[0] = 0;
        _patchLocation[1] = 0;
        _patchLocation[2] = 0;
        _patchLocation[3] = 0;
        _patchLocation[4] = 0;
        _patchLocation[5] = 0;
        // 6: keep the same
        *(const char**)((u8*)&_patchLocation[7] + 8 + (_patchLocation[7] & 0xFFF)) = _bannerSavePath;
        _patchLocation[8] = 0;
        _patchLocation[9] = 0xE3A02040; // mov r2, #64
    }
    else if (_foundSignature == sPatchTarget2)
    {
        // ldr     r1,=Lxx_#0x2FCE928
        // mov     r0,r9
        // bl      Lxx_#0x2FC8558
        // cmp     r0,#0x0
        // bne     Lxx_#0x2FC8174
        // bl      Lxx_#0x37C7654
        // cmp     r0,#0x3
        // bne     Lxx_#0x2FC8174
        // ldr     r1,=Lxx_#0x2FCE948                -> nop
        // mov     r0,r5                             -> nop
        // bl      Lxx_#0x37C834C                    -> nop
        // cmp     r0,#0x0                           -> nop
        // beq     Lxx_#0x2FC82A4                    -> nop
        // sub     r7,r0,r5          <- PatchTarget  -> nop
        // add     r9,r13,#0x66
        // mov     r0,r9
        // mov     r1,r5                             -> nop
        // add     r2,r7,#0x1                        -> nop
        // bl      Lxx_#0x2FC78CC                    -> nop
        // ldr     r1,=Lxx_#0x2FCE954                -> change the pool address to the banner save path
        // sub     r2,r4,r7                          -> mov r2, #64
        // add     r0,r9,r7,lsl #0x1                 -> nop
        // b       Lxx_#0x2FC810C
        _patchLocation[-5] = 0;
        _patchLocation[-4] = 0;
        _patchLocation[-3] = 0;
        _patchLocation[-2] = 0;
        _patchLocation[-1] = 0;
        _patchLocation[0] = 0;
        // 1: keep the same
        // 2: keep the same
        _patchLocation[3] = 0;
        _patchLocation[4] = 0;
        _patchLocation[5] = 0;
        *(const char**)((u8*)&_patchLocation[6] + 8 + (_patchLocation[6] & 0xFFF)) = _bannerSavePath;
        _patchLocation[7] = 0xE3A02040; // mov r2, #64
        _patchLocation[8] = 0;
    }
    else if (_foundSignature == sPatchTargetThumb1)
    {
        // ldr     r1,=Lxx_#0x2F992E0
        // mov     r0,r4
        // bl      Lxx_#0x2F96CD0
        // cmp     r0,#0x0
        // bne     Lxx_#0x2F96A0E
        // bl      Lxx_#0x2F96CA0
        // cmp     r0,#0x3
        // bne     Lxx_#0x2F96A0E
        // ldr     r1,=Lxx_#0x2F99300                   -> nop
        // mov     r0,r7                                -> nop
        // bl      Lxx_#0x2F96CB0                       -> nop nop
        // cmp     r0,#0x0                              -> nop
        // beq     Lxx_#0x2F96ACC                       -> nop
        // sub     r0,r0,r7            <- PatchTarget   -> nop
        // str     r0,[sp,#0x30]                        -> nop
        // add     r4,sp,#0x88
        // ldr     r2,[sp,#0x30]                        -> nop
        // add     r4,#0x2
        // mov     r0,r4
        // mov     r1,r7                                -> nop
        // add     r2,r2,#0x1                           -> nop
        // bl      Lxx_#0x2F963C8                       -> nop nop
        // ldr     r0,[sp,#0x30]                        -> nop
        // ldr     r3,=Lxx_#0x103                       -> nop
        // ldr     r2,[sp,#0x30]                        -> nop
        // lsl     r0,r0,#0x1                           -> nop
        // ldr     r1,=Lxx_#0x2F9930C                   -> change the pool address to the banner save path
        // add     r0,r4,r0                             -> nop
        // sub     r2,r3,r2                             -> movs r2, #64
        // b       Lxx_#0x2F969C2
        u16* u16PatchLocation = (u16*)_patchLocation;
        u16PatchLocation[-5] = THUMB_NOP;
        u16PatchLocation[-4] = THUMB_NOP;
        u16PatchLocation[-3] = THUMB_NOP;
        u16PatchLocation[-2] = THUMB_NOP;
        u16PatchLocation[-1] = THUMB_NOP;
        u16PatchLocation[0] = THUMB_NOP;
        u16PatchLocation[1] = THUMB_NOP;
        u16PatchLocation[2] = THUMB_NOP;
        // 3: keep the same
        u16PatchLocation[4] = THUMB_NOP;
        // 5: keep the same
        // 6: keep the same
        u16PatchLocation[7] = THUMB_NOP;
        u16PatchLocation[8] = THUMB_NOP;
        u16PatchLocation[9] = THUMB_NOP;
        u16PatchLocation[10] = THUMB_NOP;
        u16PatchLocation[11] = THUMB_NOP;
        u16PatchLocation[12] = THUMB_NOP;
        u16PatchLocation[13] = THUMB_NOP;
        u16PatchLocation[14] = THUMB_NOP;
        *(const char**)((((u32)&u16PatchLocation[15] + 4) & ~2) + (u16PatchLocation[15] & 0xFF) * 4) = _bannerSavePath;
        u16PatchLocation[16] = THUMB_NOP;
        u16PatchLocation[17] = THUMB_MOVS_IMM(THUMB_R2, 64);
    }
}
