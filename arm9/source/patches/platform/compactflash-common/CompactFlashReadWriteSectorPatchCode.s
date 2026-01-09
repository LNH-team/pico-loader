.cpu arm7tdmi
.syntax unified
.thumb

.section "cf_perform_transfer", "ax"

.equ CF_CMD_LBA, 0xE0
.equ CF_CMD_READ, 0x20
.equ CF_CMD_WRITE, 0x30

@ bool cf_performTransferSectors(u32 numSectors, u32 sector, void* srcAddr, void* dstAddr, u8 command)
.type cf_performTransferSectors, %function
.global cf_performTransferSectors
cf_performTransferSectors:
    push {r0,r1,r4-r7,lr}
    ldr r7, cf_performTransferSectors_waitCardAvailableForCommands
    @ calls waitCardAvailableForCommands
    @ this function doesn't alter r0, but sets the zero flag on failure and clears it on success
    bl cf_performTransferSectors_interwork
    beq cf_performTransferSectors_error

    ldr r5, cf_performTransferSectors_reg_sector_count

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

    ldr r7, cf_performTransferSectors_waitNextBlockReady
read_next_block:
    @ calls waitCardNextBlockReady
    @ this function doesn't alter r0, but sets the zero flag on failure and clears it on success
    bl cf_performTransferSectors_interwork
    beq cf_performTransferSectors_error

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
cf_performTransferSectors_error:
    pop {r0,r1,r4-r7, pc}

cf_performTransferSectors_interwork:
    bx r7

.balign 4
.pool
.global cf_performTransferSectors_reg_sector_count
cf_performTransferSectors_reg_sector_count:
    .word 0
.global cf_performTransferSectors_waitCardAvailableForCommands
cf_performTransferSectors_waitCardAvailableForCommands:
    .word 0
.global cf_performTransferSectors_waitNextBlockReady
cf_performTransferSectors_waitNextBlockReady:
    .word 0

.section "cf_read_write_functions", "ax"
.global cf_performTransfer_unlock_label
.global cf_performTransfer_lock_label
@ r2 srcAddr
@ r3 dstAddr
@ r4 command
@ top of stack startSector
@ below it numSectors
@ cf_performTransfer(dstAddr, srcAddr, command, startSector, numSectors)
cf_performTransfer:
    @ loads EXMEMCNT register address
    ldr r6, =0x04000200
    @ waitstate 4,2 and arm9 slot2 access
    @ r6 + 4 is EXMEMCNT, use lower 8 bits as 0
    strb r6, [r6, #4]
    
    ldr r7, cf_performTransfer_lockUnlockCard
    movs r0, #0

    @ if the cart requires no lock/unlock sequence, this is replaced with a nop
cf_performTransfer_unlock_label:
    @ calls lockUnlockCard
    bl cf_performTransfer_interwork

    @ r2,r3,r4 hold variables not to be touched
    
    ldr r7, cf_performTransfer_performTransferSectors

    @ r1 holds startSector
    @ r5 holds remainingSectors
    pop {r1,r5}
    movs r0, #0xFF
readNextSectorBlock:
    subs r5, r0
    ble lastRead

    @ calls performTransferSectors
    @ this function doesn't alter r0, but sets the zero flag on failure and clears it on success
    bl cf_performTransfer_interwork
    beq error
    @ increment sector
    adds r1, r0
    b readNextSectorBlock
    
lastRead:
    adds r0, r5
    @ calls performTransferSectors
    @ this function doesn't alter r0, but sets the zero flag on failure and clears it on success
    bl cf_performTransfer_interwork

error:    
    ldr r7, cf_performTransfer_lockUnlockCard
    movs r0, #1

    @ if the cart requires no lock/unlock sequence, this is replaced with a nop
cf_performTransfer_lock_label:
    @ calls lockUnlockCard
    bl cf_performTransfer_interwork

    @ waitstate 4,2 and arm7 slot2 access
    movs r2, #0x80
    @ r6 + 4 is EXMEMCNT
    strb r2, [r6, #4]
    pop {r4-r7, pc}
cf_performTransfer_interwork:
    bx r7

@ cf_readSectors(u32 sector, void* buffer, u32 numSectors)
.type cf_readSectors, %function
.global cf_readSectors
cf_readSectors:
    push {r0,r2,r4-r7, lr}
    movs r4, CF_CMD_READ
    movs r3, r1
    ldr r2, cf_performTransfer_reg_data

    b cf_performTransfer

@ cf_writeSectors(u32 sector, void* buffer, u32 numSectors)
.type cf_writeSectors, %function
.global cf_writeSectors
cf_writeSectors:
    push {r0,r2,r4-r7, lr}
    movs r4, CF_CMD_WRITE
    ldr r3, cf_performTransfer_reg_data
    movs r2, r1

    b cf_performTransfer

.balign 4
.pool
.global cf_performTransfer_reg_data
cf_performTransfer_reg_data:
    .word 0
.global cf_performTransfer_performTransferSectors
cf_performTransfer_performTransferSectors:
    .word 0
.global cf_performTransfer_lockUnlockCard
cf_performTransfer_lockUnlockCard:
    .word 0
