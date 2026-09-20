.cpu arm946e-s
.section "ezp_readsectorsdma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global ezp_readSectorsDma
.type ezp_readSectorsDma, %function
ezp_readSectorsDma:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r7, #0x80
    strb r7, [r4,#0x1]

sector_loop:
    // request sd read for sector 0xaabbccdd
    // for xx number of sectors (up to 4 sectors)
    // TODO implement multiple sectors
    // B9 00 00 xx aa bb cc dd
    movs r7, #0xB9
    str r7, [r4,#0x8]
    movs r7, #1
    strb r7, [r4,#0xB]
    lsrs r7, r0, #24
    strb r7, [r4,#0xC]
    lsrs r7, r0, #16
    strb r7, [r4,#0xD]
    lsrs r7, r0, #8
    strb r7, [r4,#0xE]
    strb r0, [r4,#0xF]

    ldr r1, =0x04100010

B9_poll_loop:
    ldr r7, =0xA75860C8
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

    ldr r6, ezp_readSectorsDma_miiCardDmaCopy32Ptr
    blx r6

BA_data_dma:
    movs r7, #0xC0 // select rom mode, with irq
    strb r7, [r4,#0x1]
    ldr r7, =0xA15A6000
    str r7, [r4,#0x4]

    pop {r4-r7,pc}

.balign 4

.global ezp_readSectorsDma_miiCardDmaCopy32Ptr
ezp_readSectorsDma_miiCardDmaCopy32Ptr:
    .word 0

.pool

.global ezp_finishReadSectorsDma
.type ezp_finishReadSectorsDma, %function
ezp_finishReadSectorsDma:
    // No cleanup needed on this platform.
    bx lr

.balign 4

.pool

.end
