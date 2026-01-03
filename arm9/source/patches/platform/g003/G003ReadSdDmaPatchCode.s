.cpu arm946e-s
.section "g003_readsddma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global g003_readSdDma
.type g003_readSdDma, %function
g003_readSdDma:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r7, #0x80
    strb r7, [r4,#0x1]

sector_loop:
    // request sd read for sector, sector << 1 == 0xaabbccdd
    // C9 bb cc dd 00 00 00 aa

    // left shift sector address 1
    lsls r0, r0, #1

    movs r7, #0xC9
    strb r7, [r4,#0x8]
    lsrs r7, r0, #24
    strb r7, [r4,#0xF] // place MSByte of sector address at end of command instead
    lsrs r7, r0, #16
    strb r7, [r4,#0x9]
    lsrs r7, r0, #8
    strb r7, [r4,#0xA]
    lsls r7, r0, #24
    lsrs r7, r7, #24 // r7 = dd
    // Ace3DS+ code uses str, but G003 doesn't store the last byte at u8 MCCMD1[4].
    // So we use strb just like the other bytes.
    strb r7, [r4,#0xB]

    ldr r1, =0x04100010

C9_poll_loop:
    ldr r7, =0xA7586000
    str r7, [r4,#0x4]

C9_poll_transfer_loop:
    ldrb r7, [r4,#0x6]
    lsrs r7, r7, #8
    bcc C9_poll_check_transfer_end
    ldr r5, [r1]

C9_poll_check_transfer_end:
    ldrb r7, [r4,#0x7]
    lsrs r7, r7, #8
    bcs C9_poll_transfer_loop

    cmp r5, #0
    bne C9_poll_loop

    // Setup data read card command
    movs r7, #0xCA
    str r7, [r4,#0x8]
    str r5, [r4,#0xC] // r5 is 0 here

readDataWithDma:
    movs r0, r2 // DMA channel
    movs r2, r3 // Destination
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count

    ldr r6, g003_readSdDma_miiCardDmaCopy32Ptr
    blx r6

CA_data_dma:
    movs r7, #0xC0 // select rom mode, with irq
    strb r7, [r4,#0x1]
    ldr r7, =0xA1586000
    str r7, [r4,#0x4]

    pop {r4-r7,pc}

.balign 4

.global g003_readSdDma_miiCardDmaCopy32Ptr
g003_readSdDma_miiCardDmaCopy32Ptr:
    .word 0

.pool

.global g003_finishReadSdDma
.type g003_finishReadSdDma, %function
g003_finishReadSdDma:
    // No cleanup needed on this platform.
    bx lr

.balign 4

.pool

.end
