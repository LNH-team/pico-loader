.cpu arm946e-s
.section "dstt_readsddma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global dstt_readSdDma
.type dstt_readSdDma, %function
dstt_readSdDma:
    push {r2-r7,lr}

    ldr r4, =0x040001A0

    movs r3, #0x80
    strb r3, [r4,#1]

    adr r2, dstt_romSettings
    // r2 - dstt_romSettings
    // r3 - dstt_readSdDma_applySectorCommand_address
    // r5 - dstt_readSdDma_stopTransmission_address
    // r6 - dstt_readSdDma_waitDataReady_address
    // r7 - dstt_readSdDma_miiCardDmaCopy32Ptr
    ldmia r2, {r2, r3, r5, r6, r7}

    cmp r1, #0
    beq applySector

    subs r1, r0, r1
    cmp r1, #1 // if sequential
    beq sector_loop_sequential

    blx r5 // dstt_readSdDma_stopTransmission_address

applySector:
    blx r3 // dstt_readSdDma_applySectorCommand_address

    b sector_loop

sector_loop_sequential:
    // set sd host mode DSTT_MODE_RECEIVE_DATA_BLOCK_STOP_CLOCK
    // 51 00 00 00 00 00 07 00
    movs r1, #0x51
    str r1, [r4, #0x8]
    movs r1, #7
    lsls r1, r1, #16
    str r1, [r4, #0xC]

sector_loop:
    str r2, [r4,#4]

    movs r1, #0x41
    lsls r1, r1, #20 // r1 = 0x04100000

cmd_wait_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc cmd_wait_loop

    ldr r3, [r1, #0x10]
    blx r6 // dstt_readSdDma_waitDataReady_address

    // previous push had r2 = dma channel, r3 = dst at top of stack
    // pop to the correct register for miiCardDmaCopy32
    pop {r0, r2}
    adds r1, #0x10 // Source. r1 is 0x04100000 from the calling function
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count

    blx r7 // dstt_readSdDma_miiCardDmaCopy32Ptr

cmd81_poll_dma:
    // read sd fifo
    // 81 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r1, #0x81
    strb r1, [r4,#0x8]

    movs r1, #0xC0 // select rom mode, with irq
    strb r1, [r4,#0x1]
    movs r1, #0xA1
    strb r1, [r4,#7]

    pop {r4-r7,pc}

.balign 4

dstt_romSettings:
    .word 0xA7180000

.global dstt_readSdDma_applySectorCommand_address
dstt_readSdDma_applySectorCommand_address:
    .word 0

.global dstt_readSdDma_stopTransmission_address
dstt_readSdDma_stopTransmission_address:
    .word 0

.global dstt_readSdDma_waitDataReady_address
dstt_readSdDma_waitDataReady_address:
    .word 0

.global dstt_readSdDma_miiCardDmaCopy32Ptr
dstt_readSdDma_miiCardDmaCopy32Ptr:
    .word 0

.pool

.end
