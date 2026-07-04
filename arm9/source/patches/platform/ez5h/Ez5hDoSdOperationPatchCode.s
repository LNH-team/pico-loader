#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_do_sd_operation", "ax"

.global ez5h_doSDOperation_sendSDIOCommand

@ bool ez5h_doSDOperation(uint32_t sector, void* buffer, uint32_t num_sectors, bool(*operation)(u32 sector, void* buffer))
BEGIN_ASM_FUNC ez5h_doSDOperation
	@ push r1 so that the sdio functions can take the argument from the satck and update it accordingly
	push {r1,lr}
	ldr SEND_SDIO_COMMAND_REG, ez5h_doSDOperation_sendSDIOCommand
	movs r6, r2
	@ get final sector
	adds r4, r0, r6

	@ mark the sdio callback function as thumb, this requires that this register is
	@ never passed with the thumb bit set, since we can't have a free `orr`
	adds r3,#1

check_next_sector:
	@ retrieve the current buffer address, the sdio functions will then take care of pushing the buffer+512 each subsequent call
	ldr r1, [sp]
	@ get current sector being read
	subs r0, r4, r6
	bl call_sdio_function_in_r3
	@ the sdio functions set the negative flag on error
	bmi sderror

	subs r6, #1
	bne check_next_sector

sderror:
	@ r1 is the buffer address
	@ r3 is the actual return address
	pop {r1,r3,r4-r7}

call_sdio_function_in_r3:
	bx r3

.balign 4
ez5h_doSDOperation_sendSDIOCommand:
	.word 0
