#include "asminc.h"

.syntax unified
.arm

.section "ez5h_read_write_sd", "ax"

@ez5h_writeMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ez5h_writeMultipleSector
	ldr	r3, ez5h_readWriteSector_writeSector
	b save_regs_and_switch_to_thumb

@ez5h_readMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ez5h_readMultipleSector
	ldr	r3, ez5h_readWriteSector_readSector
save_regs_and_switch_to_thumb:
	@ push r0,r1,r2,r3 so that they can be popped in the right regs below
	push {r0-r3,r4-r12,lr}
	adr r0, sdio_functions
	ldmia r0!, {SEND_SDIO_COMMAND_REG,SEND_COMMAND_REG,SEND_WRITE_DATA_ROM_REG,SDIO_CRC_REG}
	orr r0, #1
	bx r0

.thumb
.global ez5h_readWriteSector_sendSDIOCommand
.global ez5h_readWriteSector_sendCommand
.global ez5h_readWriteSector_sendWriteDataRomCommand
.global ez5h_readWriteSector_sdio4BitCrc16

sdio_functions:
ez5h_readWriteSector_sendSDIOCommand:
	.word 0
ez5h_readWriteSector_sendCommand:
	.word 0
ez5h_readWriteSector_sendWriteDataRomCommand:
	.word 0
ez5h_readWriteSector_sdio4BitCrc16:
	.word 0

@ bool doOperation(uint32_t sector, void* buffer, uint32_t num_sectors, bool(*operation)(u32 sector, void* buffer))
doSDOperation:
	@ these are the og r0,r1,r2,r3 that got pushed in the entrypoint
	pop {r4,r5,r6,r7}
	@ get final sector
	adds r4, r6

check_next_sector:
	movs r1, r5
	@ get current sector being read
	subs r0, r4, r6
	bl call_sdio_function_in_r7
	cmp r0, #0x0
	beq sderror

	@ load 0x200
	movs r3, #0x80
	lsls r3, #2
	adds r5, r3
	subs r6, #1
	bne check_next_sector

sderror:
	adr r7, return_interwork

call_sdio_function_in_r7:
	bx	r7

.balign 4
.global ez5h_readWriteSector_readSector
ez5h_readWriteSector_readSector:
	.word 0
.global ez5h_readWriteSector_writeSector
ez5h_readWriteSector_writeSector:
	.word 0

.arm
return_interwork:
	pop {r4-r12,lr}
	bx lr

