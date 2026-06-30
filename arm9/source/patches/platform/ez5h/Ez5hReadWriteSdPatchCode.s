#include "asminc.h"

.syntax unified
.thumb

.section "ez5h_read_write_sd", "ax"

@ez5h_writeMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC ez5h_writeMultipleSector
	ldr	r3, ez5h_ez5h_doSDOperation_writeSector
	b ez5h_doSDOperation

@ez5h_readMultipleSector(u32 sector, u8 * buffer, u32 num_sectors)
BEGIN_ASM_FUNC_NO_SECTION ez5h_readMultipleSector
	ldr	r3, ez5h_ez5h_doSDOperation_readSector

@ bool doOperation(uint32_t sector, uint32_t num_sectors, void* buffer, bool(*operation)(u32 sector, void* buffer))
ez5h_doSDOperation:
	push {r3-r7, lr}
	movs r4, r0
	movs r5, r2
	movs r7, r3
	adds r6, r0, r1

check_next_sector:
	cmp r4, r6
	bne parse_next_sector

	movs r0, #0x1
sderror:
	pop {r3-r7,pc}

parse_next_sector:
	movs r1, r5
	movs r0, r4
	bl call_sd_function
	cmp r0, #0x0
	beq sderror

	movs r3, #0x80
	lsls r3, #0x2
	adds r5, r3
	adds r4, #0x1
	b check_next_sector

call_sd_function:
	bx	r7

.balign 4
.global ez5h_ez5h_doSDOperation_readSector
ez5h_ez5h_doSDOperation_readSector:
	.word 0
.global ez5h_ez5h_doSDOperation_writeSector
ez5h_ez5h_doSDOperation_writeSector:
	.word 0
