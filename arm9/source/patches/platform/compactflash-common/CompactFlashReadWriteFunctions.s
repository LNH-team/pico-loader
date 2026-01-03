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

@ bool CF_PerformTransferSectors(u32 numSectors, u32 sector, void* srcAddr, void* dstAddr, u8 command)
.type CF_PerformTransferSectors, %function
.global CF_PerformTransferSectors
CF_PerformTransferSectors:
    push {r0-r2,r5-r7,lr}
    ldr r7, cf_readWriteFunctions_available_for_command
    bl CF_PerformTransferSectors_error_interwork
    beq CF_PerformTransferSectors_error

    ldr r5, cf_readWriteFunctions_reg_sector_count

    @ load 0x20000
    movs r6, #0x01
    lsls r6, #17

    @ store sector count
    strh r0, [r5]
    adds r5, r6
    
    lsls r7, r1, #24
    lsrs r7, r7, #24
    @ store lba1
    strh r7, [r5]
    adds r5, r6

    lsls r7, r1, #16
    lsrs r7, r7, #24
    @ store lba2
    strh r7, [r5]
    adds r5, r6

    lsls r7, r1, #8
    lsrs r7, r7, #24
    @ store lba3
    strh r7, [r5]
    adds r5, r6

    @ Only lower nibble is transferred
    lsls r7, r1, #4
    lsrs r7, r7, #28
    @ store lba4
    adds r7, CF_CMD_LBA
    strh r7, [r5]

    @ store command
    strh r4, [r5, r6]

    @ get total number of bytes to write
    lsls r0, #9

    ldr r7, cf_readWriteFunctions_waitCardNextBlockReady
read_next_block:
    bl CF_PerformTransferSectors_error_interwork
    beq CF_PerformTransferSectors_error

read_next_int:
    ldm r2!, {r1,r4,r5,r6}
    stm r3!, {r1,r4,r5,r6}

    subs r0, #16
    beq done
    
    @ Shifting left by 0x17 will set the Zero flag if the number that was shifted is a multiple
    @ of 0x200 (indicating a full sector has been written)
    lsls r1, r0, #0x17
    bne read_next_int

    b read_next_block
done:
    movs r0, #1
CF_PerformTransferSectors_error:
    pop {r0,r1,r3,r5-r7, pc}

CF_PerformTransferSectors_error_interwork:
    bx r7

.balign 4
.pool
.global cf_readWriteFunctions_reg_sector_count
cf_readWriteFunctions_reg_sector_count:
    .word 0
.global cf_readWriteFunctions_available_for_command
cf_readWriteFunctions_available_for_command:
    .word 0
.global cf_readWriteFunctions_waitCardNextBlockReady
cf_readWriteFunctions_waitCardNextBlockReady:
    .word 0

.section "cf_read_write_functions_2", "ax"
.global CF_PerformTransfer_unlock_label
.global CF_PerformTransfer_lock_label
@ r2 srcAddr
@ r3 dstAddr
@ r4 command
@ top of stack startSector
@ below it numSectors
@ CF_PerformTransfer(dstAddr, srcAddr, command, startSector, numSectors)
CF_PerformTransfer:
    @ loads EXMEMCNT register address
    ldr r6, =0x04000200
    @ waitstate 4,2 and arm9 slot2 access
    @ r6 + 4 is EXMEMCNT, use lower 8 bits as 0
    strb r6, [r6, #4]
    
    ldr r7, cf_readWriteFunctions2_lockUnlockCard
    movs r0, #0

    @ if the cart requires no lock/unlock sequence, this is replaced with a nop
CF_PerformTransfer_unlock_label:
    bl interwork

    @ r2,r3,r4 hold variables not to be touched
    
    ldr r7, cf_readWriteFunctions2_performTransferSectors

    @ r1 holds startSector
    @ r5 holds remainingSectors
    pop {r1,r5}
    movs r0, 0xFF
readNextSectorBlock:
    subs r5, r0
    ble lastRead

    bl interwork
    beq error
    @ increment sector
    adds r1, r0
    b readNextSectorBlock
    
lastRead:
    adds r0, r5
    bl interwork

error:    
    ldr r7, cf_readWriteFunctions2_lockUnlockCard
    movs r0, #1

    @ if the cart requires no lock/unlock sequence, this is replaced with a nop
CF_PerformTransfer_lock_label:
    bl interwork

    @ waitstate 4,2 and arm7 slot2 access
    movs r2, #0x80
    @ r6 + 4 is EXMEMCNT
    strb r2, [r6, #4]
    pop {r4-r7, pc}
interwork:
    bx r7

@ CF_readSectors(u32 sector, void* buffer, u32 numSectors)
.type CF_readSectors, %function
.global CF_readSectors
CF_readSectors:
    push {r0,r2,r4-r7, lr}
    movs r4, CF_CMD_READ
    movs r3, r1
    ldr r2, cf_readWriteFunctions2_reg_data

    b CF_PerformTransfer

@ CF_writeSectors(u32 sector, void* buffer, u32 numSectors)
.type CF_writeSectors, %function
.global CF_writeSectors
CF_writeSectors:
    push {r0,r2,r4-r7, lr}
    movs r4, CF_CMD_WRITE
    ldr r3, cf_readWriteFunctions2_reg_data
    movs r2, r1

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
