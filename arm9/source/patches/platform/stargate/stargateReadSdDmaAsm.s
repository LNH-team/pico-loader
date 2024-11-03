.cpu arm946e-s
.section "stargate_readsddma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global stargate_readSdDma
.type stargate_readSdDma, %function
stargate_readSdDma:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r7, #0x80
    strb r7, [r4,#0x1]

sector_loop:
    // request sd read for sector 0xaabbccdd
    // B9 aa bb cc dd 00 00 00
    movs r7, #0xB9
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

    ldr r1, =0x04100010

B9_poll_loop:
    ldr r7, =0xA7406000
    str r7, [r4,#0x4]

B9_poll_transfer_loop:
    ldrb r7, [r4,#0x6]
    lsrs r7, r7, #8
    bcc B9_poll_check_transfer_end
    ldr r5, [r1]

B9_poll_check_transfer_end:
    ldrb r7, [r4,#0x7]
    lsrs r7, r7, #8
    bcs B9_poll_transfer_loop

    cmp r5, #0
    bne B9_poll_loop

    // Setup data read card command
    movs r7, #0xBA
    str r7, [r4,#0x8]
    str r5, [r4,#0xC] // r5 is 0 here

readDataWithDma:
    movs r0, r2 // DMA channel
    movs r2, r3 // Destination
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count

    ldr r6, stargate_readSdDma_miiCardDmaCopy32Ptr
    blx r6

BA_data_dma:
    movs r7, #0xC0 // select rom mode, with irq
    strb r7, [r4,#0x1]
    ldr r7, =0xA1406000
    str r7, [r4,#0x4]

    pop {r4-r7,pc}

.balign 4

.global stargate_readSdDma_miiCardDmaCopy32Ptr
stargate_readSdDma_miiCardDmaCopy32Ptr:
    .word 0

.pool

.global stargate_finishReadSdDma
.type stargate_finishReadSdDma, %function
stargate_finishReadSdDma:
    // No cleanup needed on this platform.
    bx lr

.balign 4

.pool

.end
