#include "asminc.h"

.syntax unified
.arm

.section "ez5h_do_sd_operation", "ax"

.global ez5h_doSDOperation_sendSDIOCommand

@ bool ez5h_doSDOperation(uint32_t sector, void* buffer, uint32_t num_sectors, bool(*operation)(u32 sector, void* buffer))
BEGIN_ASM_FUNC ez5h_doSDOperation
	@ push r0,r1,r2 so that they can be popped in the right regs below
	@ once popped, it will leave on the stack lr,r4-r7
	push {r0,r1,r2,lr}
	ldr SEND_SDIO_COMMAND_REG, ez5h_doSDOperation_sendSDIOCommand
	@ these are the og r0,r1,r2 that got pushed in the entrypoint
	pop {r4,r5,r6}
	@ get final sector
	adds r4, r6

check_next_sector:
	movs r1, r5
	@ get current sector being read
	subs r0, r4, r6
	bl call_sdio_function_in_r3
	cmp r0, #0x0
	beq sderror

	@ load 0x200
	movs r2, #0x80
	lsls r2, #2
	adds r5, r2
	subs r6, #1
	bne check_next_sector

sderror:
	@ r3 is the actual return address
	pop {r3,r4-r7}

call_sdio_function_in_r3:
	bx r3

.balign 4
ez5h_doSDOperation_sendSDIOCommand:
	.word 0
