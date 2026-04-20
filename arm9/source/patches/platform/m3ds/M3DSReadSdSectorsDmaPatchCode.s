.cpu arm946e-s
.section "m3ds_readsdsectorsdma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global m3ds_readSdSectorsDma
.type m3ds_readSdSectorsDma, %function
m3ds_readSdSectorsDma:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r7, #0x80
    strb r7, [r4,#0x1]

    cmp r1, #0 // if first sector
    beq sector_loop // first sector, skip poll

    subs r1, r0, r1
    cmp r1, #1 // if sequential
    beq sector_loop_next_sequential

    ldr r7, m3ds_readSdSectorsDma_sdStopTransmission_address
    blx r7

sector_loop:
    // request sd read for sector 0xaabbccdd
    // B1 aa bb cc dd 00 00 00
    movs r7, #0xB1
    strb r7, [r4,#0x8]
    lsrs r7, r0, #24
    strb r7, [r4,#0x9]
    lsrs r7, r0, #16
    strb r7, [r4,#0xA]
    lsrs r7, r0, #8
    strb r7, [r4,#0xB]
    lsls r7, r0, #24
    lsrs r7, r7, #24 // r7 = dd
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    b cmd_poll_loop

sector_loop_next_sequential:
    // request sd read for next contiguous sector
    // starting from sector 0xaabbccdd
    // B2 aa bb cc dd 00 00 00
    movs r7, #0xB2
    strb r7, [r4,#0x8]

cmd_poll_loop:
    ldr r1, =0x04100010
    ldr r7, m3ds_readSdSectorsDma_applyCommand_address
    blx r7

    // Setup data read card command
    movs r7, #0xBA
    strb r7, [r4,#0x8]

readDataWithDma:
    movs r0, r2 // DMA channel
    movs r2, r3 // Destination
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count

    ldr r6, m3ds_readSdSectorsDma_miiCardDmaCopy32Ptr
    blx r6

BA_data_dma:
    movs r7, #0xC0 // select rom mode, with irq
    strb r7, [r4,#0x1]
    movs r7, #0xA1
    strb r7, [r4,#0x7]

    pop {r4-r7,pc}

.balign 4

.global m3ds_readSdSectorsDma_miiCardDmaCopy32Ptr
m3ds_readSdSectorsDma_miiCardDmaCopy32Ptr:
    .word 0

.global m3ds_readSdSectorsDma_applyCommand_address
m3ds_readSdSectorsDma_applyCommand_address:
    .word 0

.global m3ds_readSdSectorsDma_sdStopTransmission_address
m3ds_readSdSectorsDma_sdStopTransmission_address:
    .word 0

.pool


.section "m3ds_readsdsectorsdma_helper", "ax"

.global m3ds_readSdSectorsDma_applyCommand
.type m3ds_readSdSectorsDma_applyCommand, %function
m3ds_readSdSectorsDma_applyCommand:
    ldr r7, =0xA7586000
    str r7, [r4,#0x4]

cmd_poll_transfer_loop:
    ldrb r7, [r4,#0x6]
    lsrs r7, r7, #8
    bcc cmd_poll_check_transfer_end

    ldr r5, [r1]

cmd_poll_check_transfer_end:
    ldrb r7, [r4,#0x7]
    lsrs r7, r7, #8
    bcs cmd_poll_transfer_loop

    cmp r5, #0
    bne m3ds_readSdSectorsDma_applyCommand
    bx lr

.balign 4

.pool

.global m3ds_sdStopTransmission
.type m3ds_sdStopTransmission, %function
m3ds_sdStopTransmission:
    push {r4-r5,lr}
    ldr r4, =0x040001A0
    movs r5, #0xB3
    str r5, [r4, #0x8]
    movs r5, #0
    str r5, [r4, #0xC]
    ldr r5, =0xA0486100
    str r5, [r4, #0x4]

// loop until card is ready
stopSdTransmission_read_loop:
    // Check MCCNT1_ENABLE
    ldrb r5, [r4,#7]
    lsrs r5, r5, #8
    bcs stopSdTransmission_read_loop
    pop {r4-r5,pc}

.balign 4

.pool

.end
