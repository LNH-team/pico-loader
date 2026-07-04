#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_read_multiple_sector", "ax"

.global ez5h_sdhc_read_label
.global ez5h_readMultipleSector_doSDOperation

@ez5h_readMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ez5h_readMultipleSector
	@ doSDOperation will take care of handling the return
	push {r4-r7}
	@ r3 will hold the value of readSector WITHOUT the thumb bit set
	@ doSDOperation will take care of setting it
	adr r3, ez5h_readSector
	ldr r4, ez5h_readMultipleSector_doSDOperation
	bx r4

.balign 4
@ negative flag set on error
@ bool ez5h_readSector(u32 sector, void* buffer)
ez5h_readSector:
	push {r3,r4-r7,lr}

ez5h_sdhc_read_label:
	lsls r1,r0,#9

	movs r0,#0x51
	CALL_NO_INTERWORK SEND_SDIO_COMMAND_REG
	@ negative on failure
	bmi sdio_fail

	adr r2,read_sector_data
	@ r2 holds the lower word of EZ5H_CMD_SDMC_READ_DATA
	@ r3 holds REG_MCCMD0
	@ r4 holds EZ5H_CTRL_READ_512
	@ r5 holds REG_MCD1
	ldmia r2, {r2,r3,r4,r5}

	@ lower word of EZ5H_CMD_SDMC_READ_DATA
	movs r7, #0

	stmia r3!, {r2, r7}
	@ REG_MCCMD0 is incremented by 8 in the stmia, REG_MCCMD0-8 = REG_MCCNT0
	subs r3, #16

	@ use this both as counter for the loop below
	@ and as setup value for REG_MCCNT0
	movs r1, #0x80
	strb r1, [r3,#1]

	@ REG_MCCNT00 + 4 = REG_MCCNT1
	@ write EZ5H_CTRL_READ_512 to mccnt1
	str r4, [r3, #4]

	@ read the outbuffer address from the stack
	ldr r6, [sp,#24]

is_busy:
	CHECK_DATA_READY r2,r3,#4,check_busy
	@ read mcd1 status flag
	ldr r2, [r5]
	@ we're looping 0x80 times (4 bytes read at the time)
	subs r1, #1
	@ if we reached the 0x80th iteration, don't write any more data
	blt check_busy
	stmia r6!, {r2}
check_busy:
	@ read mccnt1 status flag
	ldr r2, [r3,#4]
	@ check if bit 31 is set (busy flag)
	lsrs r2, #31
	bne is_busy

	@ store the incremented buffer for the caller
	str r6, [sp,#24]

sdio_fail:
	@ r0 is the og result of ez5h_sendSDIOCommand, pass it through
	pop {r3,r4-r7,pc}
.balign 4
read_sector_data:
	.word EZ5H_CMD_SDMC_READ_DATA_LOWER_WORD
	.word REG_MCCMD0
	.word EZ5H_CTRL_READ_512
	.word REG_MCD1
ez5h_readMultipleSector_doSDOperation:
	.word 0
