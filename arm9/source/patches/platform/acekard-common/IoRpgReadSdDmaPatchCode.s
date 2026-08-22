.cpu arm946e-s
.syntax unified
.section "iorpg_readsddma", "ax"
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global iorpg_readSdDma
.type iorpg_readSdDma, %function
iorpg_readSdDma:
    push {r4-r5,lr}

    ldr r4, =0x040001A0
    movs r5, #0x80
    strb r5, [r4,#1]

    cmp r1, #0 // if first sector
    beq sendCmd18 // send CMD18

    subs r1, r0, r1
    cmp r1, #1 // check if sequential
    beq waitSdState // if so, skip CMD12 and CMD18

    // CMD12 must be run as we cannot ensure the SD card is in an
    // idle state if we are in the middle of a DMA chain
    bl iorpg_finishReadSdDma

sendCmd18:
.global iorpg_readSdDma_sdsc_shift
iorpg_readSdDma_sdsc_shift:
    lsls r5, r0, #9

    ldr r0, iorpg_readSdDma_cmd18_command
    movs r1, r5
    ldr r5, iorpg_readSdDma_sendSdioCommand_address
    blx r5

    ldr r5, iorpg_readSdDma_cardWaitReady_address
    blx r5
    b readDataWithDma

waitSdState:
    // Wait for SD state
    movs r0, #7
    ldr r5, iorpg_readSdDma_sdWaitForState_address
    blx r5

readDataWithDma:
    ldr r5, iorpg_readSdDma_dmaStartTransfer_address
    bx r5

.balign 4

.global iorpg_readSdDma_cardWaitReady_address
iorpg_readSdDma_cardWaitReady_address:
    .word 0

.global iorpg_readSdDma_dmaStartTransfer_address
iorpg_readSdDma_dmaStartTransfer_address:
    .word 0

.global iorpg_readSdDma_cmd18_command
iorpg_readSdDma_cmd18_command:
    .word 0

.global iorpg_finishReadSdDma
.type iorpg_finishReadSdDma, %function
iorpg_finishReadSdDma:
    push {r0-r2,r4,lr}

    ldr r4, =0x040001A0

    // Wait for SD state
    movs r0, #7
    ldr r2, iorpg_readSdDma_sdWaitForState_address
    blx r2

    // Send CMD12 == STOP_TRANSMISSION
    ldr r0, iorpg_readSdDma_cmd12_command
    movs r1, #0
    ldr r2, iorpg_readSdDma_sendSdioCommand_address
    blx r2

    pop {r0-r2,r4,pc}

.balign 4

.global iorpg_readSdDma_sendSdioCommand_address
iorpg_readSdDma_sendSdioCommand_address:
    .word 0

.global iorpg_readSdDma_sdWaitForState_address
iorpg_readSdDma_sdWaitForState_address:
    .word 0

.global iorpg_readSdDma_cmd12_command
iorpg_readSdDma_cmd12_command:
    .word 0

.pool

.section "iorpg_dmastarttransfer", "ax"

// r2 = dma channel
// r3 = dst
.global iorpg_dmaStartTransfer
.type iorpg_dmaStartTransfer, %function
iorpg_dmaStartTransfer:
    movs r0, r2 // DMA channel
    ldr r1, =0x04100010
    movs r2, r3 // Destination
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count

    ldr r5, iorpg_dmaStartTransfer_miiCardDmaCopy32Ptr
    blx r5

    // read sectors
    movs r5, #0xB7
    str r5, [r4,#0x8]
    // The full CMD is B7 00 00 00 00 13 00 00
    // If we use 0x1300 and str, then it goes into the expected place
    movs r5, #0x13
    lsls r5, r5, #8
    str r5, [r4,#0xC]

    movs r5, #0xC0 // select rom mode, with irq
    strb r5, [r4,#0x1]
    ldr r5, =0xA1406004
    str r5, [r4,#4]

    pop {r4-r5,pc}

.balign 4

.global iorpg_dmaStartTransfer_miiCardDmaCopy32Ptr
iorpg_dmaStartTransfer_miiCardDmaCopy32Ptr:
    .word 0

.pool

.end
