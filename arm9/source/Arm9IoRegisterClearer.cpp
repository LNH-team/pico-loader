#include "common.h"
#include <libtwl/gfx/gfx3d.h>
#include <libtwl/gfx/gfxWindow.h>
#include <libtwl/rtos/rtosIrq.h>
#include <libtwl/ipc/ipcFifo.h>
#include <libtwl/ipc/ipcSync.h>
#include <libtwl/timer/timer.h>
#include <libtwl/dma/dmaNitro.h>
#include <libtwl/dma/dmaTwl.h>
#include <libtwl/mem/memVram.h>
#include <libtwl/card/card.h>
#include "Arm9IoRegisterClearer.h"

void Arm9IoRegisterClearer::ClearNtrIoRegisters(bool isSdkResetSystem) const
{
    REG_IME = 0;
    REG_IE = 0;
    // Important! Make sure that all sources of interrupts are disabled as much as possible.
    // It is possible to accidentally trigger an irq too early in games if bits are set in REG_IF
    // while it doesn't expect that.
    ClearGraphicsRegisters(isSdkResetSystem); // vblank, hblank, vcount, gxfifo
    ClearTimerRegisters(); // timer 0, timer 1, timer 2, timer 3
    ClearNtrDmaRegisters(); // dma 0, dma 1, dma 2, dma 3
    REG_MCCNT0 = 0; // rom transfer
    ipc_disableRecvFifoNotEmptyIrq();
    ipc_disableSendFifoEmptyIrq();
    ipc_disableArm7Irq();
    ipc_clearSendFifo();
    ipc_disableFifo();
    REG_KEYCNT = 0;
}

void Arm9IoRegisterClearer::ClearTwlIoRegisters() const
{
    REG_NDMA0SAD = 0;
    REG_NDMA0DAD = 0;
    REG_NDMA0TCNT = 0;
    REG_NDMA0WCNT = 0;
    REG_NDMA0BCNT = 0;
    REG_NDMA0FDATA = 0;
    REG_NDMA0CNT = 0;
    REG_NDMA1SAD = 0;
    REG_NDMA1DAD = 0;
    REG_NDMA1TCNT = 0;
    REG_NDMA1WCNT = 0;
    REG_NDMA1BCNT = 0;
    REG_NDMA1FDATA = 0;
    REG_NDMA1CNT = 0;
    REG_NDMA2SAD = 0;
    REG_NDMA2DAD = 0;
    REG_NDMA2TCNT = 0;
    REG_NDMA2WCNT = 0;
    REG_NDMA2BCNT = 0;
    REG_NDMA2FDATA = 0;
    REG_NDMA2CNT = 0;
    REG_NDMA3SAD = 0;
    REG_NDMA3DAD = 0;
    REG_NDMA3TCNT = 0;
    REG_NDMA3WCNT = 0;
    REG_NDMA3BCNT = 0;
    REG_NDMA3FDATA = 0;
    REG_NDMA3CNT = 0;
}

void Arm9IoRegisterClearer::ClearGraphicsRegisters(bool isSdkResetSystem) const
{
    REG_POWERCNT = 0x820F;
    REG_DISPSTAT = 0; // vblank, hblank, vcount
    if (!isSdkResetSystem)
    {
        REG_DISPCNT = 0;
        REG_DISPCNT_SUB = 0;
        REG_BG0CNT = 0;
        REG_BG0CNT_SUB = 0;
        REG_BG1CNT = 0;
        REG_BG1CNT_SUB = 0;
        REG_BG2CNT = 0;
        REG_BG2CNT_SUB = 0;
        REG_BG3CNT = 0;
        REG_BG3CNT_SUB = 0;
        REG_BG0HOFS = 0;
        REG_BG0HOFS_SUB = 0;
        REG_BG0VOFS = 0;
        REG_BG0VOFS_SUB = 0;
        REG_BG1HOFS = 0;
        REG_BG1HOFS_SUB = 0;
        REG_BG1VOFS = 0;
        REG_BG1VOFS_SUB = 0;
        REG_BG2HOFS = 0;
        REG_BG2HOFS_SUB = 0;
        REG_BG2VOFS = 0;
        REG_BG2VOFS_SUB = 0;
        REG_BG3HOFS = 0;
        REG_BG3HOFS_SUB = 0;
        REG_BG3VOFS = 0;
        REG_BG3VOFS_SUB = 0;
        REG_BG2PA = 0;
        REG_BG2PB = 0;
        REG_BG2PC = 0;
        REG_BG2PD = 0;
        REG_BG2X = 0;
        REG_BG2Y = 0;
        REG_BG3PA = 0;
        REG_BG3PB = 0;
        REG_BG3PC = 0;
        REG_BG3PD = 0;
        REG_BG3X = 0;
        REG_BG3Y = 0;
        REG_BG2PA_SUB = 0;
        REG_BG2PB_SUB = 0;
        REG_BG2PC_SUB = 0;
        REG_BG2PD_SUB = 0;
        REG_BG2X_SUB = 0;
        REG_BG2Y_SUB = 0;
        REG_BG3PA_SUB = 0;
        REG_BG3PB_SUB = 0;
        REG_BG3PC_SUB = 0;
        REG_BG3PD_SUB = 0;
        REG_BG3X_SUB = 0;
        REG_BG3Y_SUB = 0;
        gfx_setWindow0(0, 0, 0, 0);
        gfx_setWindow1(0, 0, 0, 0);
        gfx_setSubWindow0(0, 0, 0, 0);
        gfx_setSubWindow1(0, 0, 0, 0);
        REG_WININ = 0;
        REG_WININ_SUB = 0;
        REG_WINOUT = 0;
        REG_WINOUT_SUB = 0;
        REG_MOSAIC = 0;
        REG_MOSAIC_SUB = 0;
        REG_BLDCNT = 0;
        REG_BLDCNT_SUB = 0;
        REG_BLDALPHA = 0;
        REG_BLDALPHA_SUB = 0;
        REG_BLDY = 0;
        REG_BLDY_SUB = 0;
        REG_MASTER_BRIGHT = 0;
        REG_MASTER_BRIGHT_SUB = 0;
    }
    REG_DISP3DCNT = 0;
    REG_DISPCAPCNT = 0;
    REG_GXSTAT = 0; // gxfifo
    REG_CLEAR_COLOR = 0;
    REG_CLEAR_DEPTH = 0x7FFF;
    REG_DISP_1DOT_DEPTH = 0x7FFF;
    REG_CLRIMAGE_OFFSET = 0;
    REG_FOG_COLOR = 0;
    REG_FOG_OFFSET = 0;
    REG_ALPHA_TEST_REF = 0;
    // VRAM A used for arm9 code
    mem_setVramBMapping(MEM_VRAM_AB_NONE);
    // VRAM C used for arm7 code
    mem_setVramDMapping(MEM_VRAM_D_NONE);
    mem_setVramEMapping(MEM_VRAM_E_NONE);
    mem_setVramFMapping(MEM_VRAM_FG_NONE);
    mem_setVramGMapping(MEM_VRAM_FG_NONE);
    mem_setVramHMapping(MEM_VRAM_H_NONE);
    mem_setVramIMapping(MEM_VRAM_I_NONE);
}

void Arm9IoRegisterClearer::ClearTimerRegisters() const
{
    REG_TM0CNT_H = 0; // timer 0
    REG_TM0CNT_L = 0;
    REG_TM1CNT_H = 0; // timer 1
    REG_TM1CNT_L = 0;
    REG_TM2CNT_H = 0; // timer 2
    REG_TM2CNT_L = 0;
    REG_TM3CNT_H = 0; // timer 3
    REG_TM3CNT_L = 0;
}

void Arm9IoRegisterClearer::ClearNtrDmaRegisters() const
{
    REG_DMA0CNT = 0; // dma 0
    REG_DMA0SAD = 0;
    REG_DMA0DAD = 0;
    REG_DMA1CNT = 0; // dma 1
    REG_DMA1SAD = 0;
    REG_DMA1DAD = 0;
    REG_DMA2CNT = 0; // dma 2
    REG_DMA2SAD = 0;
    REG_DMA2DAD = 0;
    REG_DMA3CNT = 0; // dma 3
    REG_DMA3SAD = 0;
    REG_DMA3DAD = 0;
}
