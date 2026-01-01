.cpu arm7tdmi
.syntax unified
.thumb
.section "cf_read_write_functions", "ax"

.equ CF_STS_INSERTED, 0x50
.equ CF_STS_READY, 0x58
.equ CF_STS_BUSY, 0x80

.equ CF_CMD_LBA, 0xE0
.equ CF_CMD_READ, 0x20
.equ CF_CMD_WRITE, 0x30

.equ CF_CARD_TIMEOUT, 10000000

@ bool CF_PerformTransferSectors(u32 numSectors, u32 sector, u8 command, void* srcAddr, void* dstAddr)
.type CF_PerformTransferSectors, %function
.global CF_PerformTransferSectors
CF_PerformTransferSectors:
	push {r0-r7,lr}
	ldr r7, cf_readWriteFunctions_available_for_command
	bl CF_PerformTransferSectors_error_interwork
	beq CF_PerformTransferSectors_error
	
	adr r7, cf_readWriteFunctions_reg_sector_count
	ldm r7!, {r1,r2,r3,r4,r5,r6}
	strh r0, [r1]
	
	@ r0 is sector
	ldr r0, [sp, #4]
	lsls r7, r0, #24
	lsrs r7, r7, #24
	strh r7, [r2]

	lsls r7, r0, #16
	lsrs r7, r7, #24
	strh r7, [r3]

	lsls r7, r0, #8
	lsrs r7, r7, #24
	strh r1, [r4]

	lsrs r7, r7, #24
	movs r3, CF_CMD_LBA
	orrs r7, r3
	strh r7, [r5]

	@ r0 is numSectors
	@ r1 is sector
	@ r2 is command
	@ r3 is srcAddr
	@ r4 is dstAddr
	pop {r0-r4}
	strh r2, [r6]
	@ get total number of bytes to write
	lsls r0, #9

read_next_block:
	ldr r7, cf_readWriteFunctions_waitCardNextBlockReady
	bl CF_PerformTransferSectors_error_interwork
	beq CF_PerformTransferSectors_error

read_next_int:
	ldm r3!, {r1,r5,r6,r7}
	stm r4!, {r1,r5,r6,r7}

	subs r0, #16
	beq done
	
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
	lsls r1, r0, #0x17
	bne read_next_int

	b read_next_block
done:
	pop {r5-r7, pc}
CF_PerformTransferSectors_error:
	pop {r0-r7, pc}

CF_PerformTransferSectors_error_interwork:
	bx r7

.balign 4
.pool
.global cf_readWriteFunctions_reg_sector_count
cf_readWriteFunctions_reg_sector_count:
	.word 0
.global cf_readWriteFunctions_reg_lba1
cf_readWriteFunctions_reg_lba1:
	.word 0
.global cf_readWriteFunctions_reg_lba2
cf_readWriteFunctions_reg_lba2:
	.word 0
.global cf_readWriteFunctions_reg_lba3
cf_readWriteFunctions_reg_lba3:
	.word 0
.global cf_readWriteFunctions_reg_lba4
cf_readWriteFunctions_reg_lba4:
	.word 0
.global cf_readWriteFunctions_reg_command
cf_readWriteFunctions_reg_command:
	.word 0
.global cf_readWriteFunctions_available_for_command
cf_readWriteFunctions_available_for_command:
	.word 0
.global cf_readWriteFunctions_waitCardNextBlockReady
cf_readWriteFunctions_waitCardNextBlockReady:
	.word 0

.section "cf_read_write_functions_2", "ax"
@ CF_PerformTransfer(u32 numSectors, u32 sector, u8 command, void* srcAddr, void* dstAddr)
CF_PerformTransfer:
    @ loads EXMEMCNT register address
    ldr r7,= 0x04000200
    @ waitstate 4,2 and arm9 slot2 access
    @ r7 holds the EXMEMCNT address, use lower 8 bits as 0
    strb r7, [r7, #4]
	@ sector counter
	movs r6, r1

	@ remaining sectors
	movs r5, r0
	
	ldr r7, cf_readWriteFunctions2_lockUnlockCard
	movs r0, #0
	bl interwork

	@ leave r2,r3,r4 untouched, r0-r1 are thrashed, only available regs are r5,r6,r7
	
	ldr r7, cf_readWriteFunctions2_performTransferSectors

readNextSectorBlock:
	cmp r5, 0xFF
	blt lastRead

	movs r0, 0xFF
	movs r1, r6
	adds r6, r0
	subs r5, r0
	bl interwork
	beq error
	b readNextSectorBlock
	
lastRead:
	movs r0, r5
	movs r1, r6
	bl interwork

error:
	
	ldr r7, cf_readWriteFunctions2_lockUnlockCard
	movs r0, #1
	bl interwork

    @ waitstate 4,2 and arm7 slot2 access
    movs r2, #0x80
    ldr r7,= 0x04000200
    strb r2, [r7, #4]
	pop {r4-r7, pc}
interwork:
	bx r7

@ CF_readSectors(u32 sector, void* buffer, u32 numSectors)
.type CF_readSectors, %function
.global CF_readSectors
CF_readSectors:
	push {r4-r7, lr}

	ldr r3, cf_readWriteFunctions2_reg_data
	movs r4, r1
	movs r1, r0
	movs r0, r2

	movs r2, CF_CMD_READ

	b CF_PerformTransfer

@ CF_writeSectors(u32 sector, void* buffer, u32 numSectors)
.type CF_writeSectors, %function
.global CF_writeSectors
CF_writeSectors:
	push {r4-r7, lr}

	movs r3, r1
	ldr r4, cf_readWriteFunctions2_reg_data
	movs r1, r0
	movs r0, r2

	movs r2, CF_CMD_WRITE

	b CF_PerformTransfer

.balign 4
.pool
.global cf_readWriteFunctions2_reg_data
cf_readWriteFunctions2_reg_data:
	.word 0
.global cf_readWriteFunctions2_performTransferSectors
cf_readWriteFunctions2_performTransferSectors:
	.word 0
.global cf_readWriteFunctions2_lockUnlockCard
cf_readWriteFunctions2_lockUnlockCard:
	.word 0
