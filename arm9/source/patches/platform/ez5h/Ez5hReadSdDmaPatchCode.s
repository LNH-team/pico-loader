#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_read_dma", "ax"

.global ez5h_readDma_sendSDIOCommand
.global ez5h_readDma_miiCardDmaCopy32Ptr
.global ez5h_sdhc_readDma_label

@ez5h_readDma(u32 sector, u32 prevSector, u32 channel, void* dst)
BEGIN_ASM_FUNC ez5h_readDma
	push {r2-r3,r4-r7,lr}
	adr r2, read_dma_data
	@ r2 holds the lower word of EZ5H_CMD_SDMC_READ_DATA
	@ r4 holds miiCardDmaCopy32Ptr
	@ r5 holds REG_MCCMD0
	@ r6 holds EZ5H_CTRL_READ_512
	@ r7 holds sendSDIOCommand
	ldmia r2, {r2,r4,r5,r6,r7}

ez5h_sdhc_readDma_label:
	lsls r1,r0,#9

	movs r0,#0x51
	@ call sendSDIOCommand
	blx r7

	@ negative on failure
	bmi sdio_fail

	@ lower word of EZ5H_CMD_SDMC_READ_DATA
	movs r3, #0

	stmia r5!, {r2, r3}
	@ REG_MCCMD0 is incremented by 8 in the stmia, REG_MCCMD0-8 = REG_MCCNT0
	subs r5, #16

    @ r0 // DMA channel
    @ r2 // Destination
	pop {r0,r2}
	ldr r1, =REG_MCD1
	@ (1 << 9) = 512 = count
    movs r3, #1
    lsls r3, r3, #9

	@ call miiCardDmaCopy32Ptr
    blx r4

	@ select rom mode, with irq
    movs r7, #0xC0 
    strb r7, [r5,#0x1]
	@ write EZ5H_CTRL_READ_512 to mccnt1
    str r6, [r5,#0x4]

	pop {r4-r7,pc}
sdio_fail:
	pop {r2-r3,r4-r7,pc}

BEGIN_ASM_FUNC ez5h_endDma
	bx lr

.balign 4
.pool
read_dma_data:
	.word EZ5H_CMD_SDMC_READ_DATA_LOWER_WORD
ez5h_readDma_miiCardDmaCopy32Ptr:
	.word 0
	.word REG_MCCMD0
	.word EZ5H_CTRL_READ_512
ez5h_readDma_sendSDIOCommand:
    .word 0


