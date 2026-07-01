#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_read_sector", "ax"

.global ez5h_sdhc_read_label

@ bool ez5h_readSector(u32 sector, void* buffer)
BEGIN_ASM_FUNC ez5h_readSector
	push {r4-r7,lr}
	movs r6,r1

	adr r2,read_sector_data
	@ r2 holds the lower word of EZ5H_CMD_SDMC_READ_DATA
	@ r3 holds REG_MCCMD0
	@ r4 holds EZ5H_CTRL_READ_512
	@ r5 holds REG_MCD1
	ldmia r2, {r2,r3,r4,r5}

ez5h_sdhc_read_label:
	lsls r1,r0,#9

	movs r0,#0x51
	CALL_NO_INTERWORK SEND_SDIO_COMMAND_REG
	@ zero flag is set accordingly
	beq sdio_fail

	@ lower word of EZ5H_CMD_SDMC_READ_DATA
	movs r7, #0

	stmia r3!, {r2, r7}
	@ REG_MCCMD0 is incremented by 8 in the stmia, REG_MCCMD0-8 = REG_MCCNT0
	subs r3, #16

	@ REG_MCCMD0 is 0x040001A0, << 10 = 0xXXXX8000
	lsls r1, r3, #10
	strh r1, [r3]

	@ REG_MCCNT00 + 4 = REG_MCCNT1
	@ write EZ5H_CTRL_READ_512 to mccnt1
	str r4, [r3, #4]	

	@ read data
	movs r2, #0x80
	lsls r2, r2, #2
	adds r1, r6, r2

is_busy:
	CHECK_DATA_READY r2,r3,#4,check_busy
	@ read mcd1 status flag
	ldr r2, [r5]
	cmp r1, r6
	bls check_busy
	stmia r6!, {r2}
check_busy:
	@ read mccnt1 status flag
	ldr r2, [r3,#4]
	@ check if bit 31 is set (busy flag)
	cmp r2, #0
	blt is_busy

sdio_fail:
	@ r0 is the og result of ez5h_sendSDIOCommand, pass it through
	pop	 {r4-r7,pc}
.balign 4
read_sector_data:
	.word EZ5H_CMD_SDMC_READ_DATA_LOWER_WORD
	.word REG_MCCMD0
	.word EZ5H_CTRL_READ_512
	.word REG_MCD1
