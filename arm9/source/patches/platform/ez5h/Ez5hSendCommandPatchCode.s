#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_send_command", "ax"

@ returns in r2, doesn't touch other regs
@ ez5h_sendCommand(u32 byteswapped_low, u32 non_byteswapped_high) -> u8
BEGIN_ASM_FUNC ez5h_sendCommand
	push {r1,r3-r5}
	
	adr r2, ez5h_sendCommand_data
	@ r3 holds REG_MCCMD0
	@ r4 holds EZ5H_CTRL_READ_4B
	@ r5 holds REG_MCD1
	ldmia r2!, {r3,r4,r5}

	@ write card command, r0 is already byteswapped, r1 no
	stmia r3!, {r0}
	strb r1, [r3, #3]
	lsrs r1, #8
	strb r1, [r3, #2]
	lsrs r1, #8
	strb r1, [r3, #1]
	lsrs r1, #8
	strb r1, [r3, #0]
	@ REG_MCCMD0 is incremented by 8 in the stmia, REG_MCCMD0-8 = REG_MCCNT0
	subs r3, #12
	@ REG_MCCNT0 is 0x040001A0, << 10 = 0xXXXX8000
	lsls r2, r3, #10
	strh r2, [r3]

	@ REG_MCCNT0 + 4 = REG_MCCNT1
	@ write EZ5H_CTRL_READ_4B to mccnt1
	str r4, [r3, #4]	

1:
	CHECK_DATA_READY r1,r3,#4,1b

	@ read from REG_MCD1
	ldr r2, [r5]
	pop {r1,r3-r5}
	mov pc, lr

@ ez5h_sendSDIOCommand(u8 command, u32 parameter)
@ returns either 0 or EZ5H_CMD_SDMC_SEND_CLK(1) (r0-r1), thrashes r7
BEGIN_ASM_FUNC ez5h_sendSDIOCommand
	push {r2-r6,lr}
	lsls r2, r0, #24
	@ fixed part of the EZ5H_CMD_SDMC_SDIO command
	ldr r7, =0x0000FAB8
	@ this is equivalent to an OR, since the values don't overlap, but we need an ADD instruction to use 3 regs
	adds r0, r7, r2
	@ r1 is passed as is, not byteswapped, while r0 is constructed already byteswapped
	bl ez5h_sendCommand
	@ load 0x10000
	movs r4, #1
	lsls r4, #16
	@ r7 holds 0x0000FAB8
	@ this gives us the byteswapped card command equivalent to EZ5H_CMD_SDMC_PARAM_CARD(1, 0, 0), which is EZ5H_CMD_SDMC_SEND_CLK(1)
	@ with r0 being 0x0001FAB8 and r1 being 0x00000000
	adds r0, r4, r7
	@ lower part of the card command
	movs r1, #0

	movs r5, #0xFF
wait_for_start_marker:
	@ ez5h_sendCommand leaves r0-r1 intact and returns in r2
	bl ez5h_sendCommand
	tst r2, r5
	bne start_marker_not_received
	@ r0 is non-0
	b end
start_marker_not_received:
	subs r4, #1
	bne wait_for_start_marker

end:
	@ if r4 is 0, timeout expired thus function failed
	movs r0, r4
	pop {r2-r7}
	mov pc, r7

.balign 4
ez5h_sendCommand_data:
	.word   REG_MCCMD0
	.word   EZ5H_CTRL_READ_4B
	.word   REG_MCD1