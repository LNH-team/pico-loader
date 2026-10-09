#include "common.h"
extern u32 retailReturnCardiMode;
extern u32 retailReturnHeapBeforeLargest;
extern u32 retailReturnPatchStage;
extern u32 retailReturnFailedAllocation;
extern u32 retailReturnWaitHookCount;
extern u32 retailReturnArmWaitHookCount;
extern u32 retailReturnAutoloadWaitHookCount;
extern u32 retailReturnHaltSiteCount;
extern u32 retailReturnHaltHookCount;
extern u32 retailReturnWaitSite;
extern u32 retailReturnWaitStub;
extern u32 retailReturnWaitState;
extern u32 retailReturnNearbyTotal;
extern u32 retailReturnNearbyMax;
#include <libtwl/mem/memVram.h>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxStatus.h>
#include <libtwl/gfx/gfxPalette.h>
#include <libtwl/gfx/gfxBackground.h>
#include "nitroFont2.h"
#include "font_nft2.h"
#include "fastClear.h"
#include "ErrorDisplay.h"

static void waitForVBlank()
{
    while (gfx_getVCount() != 191);
    while (gfx_getVCount() == 191);
}

static bool readAButton()
{
    return ((~REG_KEYINPUT) & KEY_A) != 0;
}

void ErrorDisplay::Print(const char* errorString, bool pressAToContinue)
{
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    auto textBuffer = (u8*)0x02100000;
    memset(textBuffer, 0, 256 * 192);
    nft2_unpack((nft2_header_t*)font_nft2);
    nft2_string_render_params_t renderParams =
    {
        x: 0,
        y: 0,
        width: 256,
        height: 192
    };
    nft2_renderString((const nft2_header_t*)font_nft2, errorString, textBuffer, 256, &renderParams);
    memcpy((void*)GFX_BG_MAIN, textBuffer, 256 * 192);
    waitForVBlank();
    // 4 bit grayscale palette
    for (int i = 0; i < 16; i++)
    {
        int gray = i * 2 + (i == 0 ? 0 : 1);
        GFX_PLTT_BG_MAIN[i] = gray | (gray << 5) | (gray << 10);
    }
    REG_BG3PA = 256;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3PD = 256;
    REG_BG3X = 0;
    REG_BG3Y = 0;
    REG_BG3CNT = (1 << 7) | (1 << 14);
    REG_BLDCNT = 0;
    REG_DISPCNT = 3 | (1 << 11) | (1 << 16);
    REG_MASTER_BRIGHT = 0;
    GFX_PLTT_BG_SUB[0] = 0;
    REG_MASTER_BRIGHT_SUB = 0x8010;
    REG_DISPCNT_SUB = 0x10000;
    if (pressAToContinue)
    {
        waitForVBlank();
        bool previousAButtonState = readAButton();
        while (true)
        {
            waitForVBlank();
            bool aButtonState = readAButton();
            if (!previousAButtonState && aButtonState)
            {
                break;
            }
            previousAButtonState = aButtonState;
        }
    }
    else
    {
        while (true);
    }

    REG_MASTER_BRIGHT = 0x4010;
    REG_MASTER_BRIGHT_SUB = 0x4010;
    REG_DISPCNT = 0;
    REG_DISPCNT_SUB = 0;
    mem_setVramEMapping(MEM_VRAM_E_LCDC);
    fastClear((void*)0x06880000, 0x10000);
    fastClear((void*)GFX_PLTT_BG_MAIN, 512);
    fastClear((void*)GFX_PLTT_BG_SUB, 512);
}

namespace
{
const u8 patchLetters[26][7] = {
    {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30},
    {14,17,16,16,16,17,14}, {30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
    {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14}, {7,2,2,2,18,18,12},
    {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17},
    {14,17,17,17,17,17,14}, {30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
    {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14}, {17,17,17,17,17,10,4},
    {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31}
};
const u8 patchDigits[10][7] = {
    {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31}, {30,1,1,14,1,1,30},
    {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
    {14,16,16,30,17,17,14}, {31,1,2,4,8,8,8},
    {14,17,17,14,17,17,14}, {14,17,17,15,1,1,14}
};

void drawPatchCharacter(char c, int x, int y)
{
    const u8* glyph = nullptr;
    if (c >= 'A' && c <= 'Z')
        glyph = patchLetters[c - 'A'];
    else if (c >= '0' && c <= '9')
        glyph = patchDigits[c - '0'];
    if (!glyph)
        return;
    for (int row = 0; row < 7; row++)
        for (int col = 0; col < 5; col++)
            if (glyph[row] & (1 << (4 - col)))
                for (int dy = 0; dy < 2; dy++)
                    for (int dx = 0; dx < 2; dx++)
                    {
                        int px = x + col * 2 + dx;
                        int py = y + row * 2 + dy;
                        if (px >= 256 || py >= 192)
                            continue;
                        // VRAM pixels are always written as halfwords.
                        vu16* pixel = (vu16*)GFX_BG_MAIN + py * 128 + (px >> 1);
                        u16 mask = (px & 1) ? 0xFF00 : 0x00FF;
                        u16 white = (px & 1) ? 0x0F00 : 0x000F;
                        *pixel = (*pixel & ~mask) | white;
                    }
}

int drawPatchText(const char* text, int x, int y)
{
    while (*text)
    {
        drawPatchCharacter(*text++, x, y);
        x += 12;
    }
    return x;
}

void drawPatchValue(const char* label, u32 value, int y)
{
    int x = drawPatchText(label, 8, y);
    char digits[10];
    u32 count = 0;
    do
    {
        digits[count++] = '0' + value % 10;
        value /= 10;
    } while (value && count < sizeof(digits));
    while (count)
    {
        drawPatchCharacter(digits[--count], x, y);
        x += 12;
    }
}
}

void ErrorDisplay::PrintPatchSpaceDiagnostic(u32 beforeTotal, u32 beforeLargest,
    u32 afterTotal, u32 afterLargest)
{
    mem_setVramEMapping(MEM_VRAM_E_MAIN_BG_00000);
    fastClear((void*)GFX_BG_MAIN, 0x10000);
    if (retailReturnFailedAllocation)
    {
        drawPatchText("TEST 137 ALLOC FAIL", 8, 4);
        drawPatchValue("CPU STAGE ", retailReturnPatchStage, 28);
        drawPatchValue("NEEDED ", retailReturnFailedAllocation, 52);
        drawPatchValue("FREE TOTAL ", afterTotal, 76);
        drawPatchValue("FREE MAX ", afterLargest, 100);
        drawPatchText("PHOTO THEN REBOOT", 8, 156);
    }
    else
    {
        drawPatchText("TEST 137 BOOT MODE", 8, 4);
        drawPatchValue("RETURN MODE ", retailReturnCardiMode, 24);
        drawPatchValue("BEFORE MAX ", retailReturnHeapBeforeLargest, 44);
        drawPatchValue("AFTER TOTAL ", afterTotal, 64);
        drawPatchValue("THUMB HOOKS ", retailReturnWaitHookCount, 84);
        drawPatchValue("ARM HOOKS ", retailReturnArmWaitHookCount, 104);
        drawPatchValue("HALT SITES ", retailReturnHaltSiteCount, 124);
        drawPatchValue("HALT HOOKS ", retailReturnHaltHookCount, 144);
        drawPatchText("PHOTO THEN PRESS A", 8, 168);
    }

    waitForVBlank();
    GFX_PLTT_BG_MAIN[0] = 0;
    GFX_PLTT_BG_MAIN[15] = 0x7FFF;
    REG_BG3PA = 256;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3PD = 256;
    REG_BG3X = 0;
    REG_BG3Y = 0;
    REG_BG3CNT = (1 << 7) | (1 << 14);
    REG_BLDCNT = 0;
    REG_DISPCNT = 3 | (1 << 11) | (1 << 16);
    REG_MASTER_BRIGHT = 0;
    GFX_PLTT_BG_SUB[0] = 0;
    REG_MASTER_BRIGHT_SUB = 0x8010;
    REG_DISPCNT_SUB = 0x10000;
    if (retailReturnFailedAllocation)
    {
        // A cannot recover a failed mandatory patch. Keep the evidence visible.
        while (true)
            waitForVBlank();
    }
    bool previousA = readAButton();
    do
    {
        waitForVBlank();
        bool currentA = readAButton();
        if (currentA && !previousA)
            break;
        previousA = currentA;
    } while (true);

    REG_MASTER_BRIGHT = 0x4010;
    REG_MASTER_BRIGHT_SUB = 0x4010;
    REG_DISPCNT = 0;
    REG_DISPCNT_SUB = 0;
    mem_setVramEMapping(MEM_VRAM_E_LCDC);
    fastClear((void*)0x06880000, 0x10000);
    fastClear((void*)GFX_PLTT_BG_MAIN, 512);
    fastClear((void*)GFX_PLTT_BG_SUB, 512);
}
