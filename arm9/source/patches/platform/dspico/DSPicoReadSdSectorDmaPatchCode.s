.cpu arm946e-s
.section "dspico_readsdsectordma", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = previous src sector
// r2 = dma channel
// r3 = dst
.global dspico_readSdSectorDma
.type dspico_readSdSectorDma, %function
dspico_readSdSectorDma:
    push {r2,r3,r4,r5,r6,r7,lr}
    adr r5, readRequestSettings
    // readRequestSettings, regBase, cardDataRegAddress, miiCardDmaCopy32Ptr, pollSdDataReadyPtr
    ldmia r5, {r2,r4,r5,r6,r7}
    movs r3, #0x80
    strb r3, [r4, #0x9] // select rom mode, without irq

    cmp r1, #0 // if first sector
    beq 1f // first sector, skip poll
    blx r7 // pollSdDataReady
1:
    subs r1, r0, r1
    cmp r1, #1 // if sequential
    beq readDataWithDma

    // card_romSetCmd(0xE300000000000000ull | sector);
    movs r3, #0xE3
    str r3, [r4, #0x10] // E3 00 00 00
    movs r3, r0
    rors r3, r4 // ror 24
    str r3, [r4, #0x14] // AA XX CC XX
    strb r0, [r4, #0x17] // AA XX CC DD
    lsrs r3, r0, #16
    strb r3, [r4, #0x15]

    // card_romStartXfer
    str r2, [r4, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

    blx r7 // pollSdDataReady

readDataWithDma:
    pop {r0,r2} // dma channel, dst
    movs r1, r5 // src = 0x04100010
    movs r3, #1
    lsls r3, r3, #9 // (1 << 9) = 512 = count
    blx r6

3:
    ldrb r0, [r4, #0xF]
    lsrs r0, r0, #8
    bcs 3b
    // r0 is zero here

    str r0, [r4, #0x14] // 00 00 00 00
    movs r0, #0xE5
    str r0, [r4, #0x10] // E5 00 00 00

    ldr r0,= 0xA1444000

    movs r1, #0xC0
    strb r1, [r4, #0x9] // select rom mode, with irq

    str r0, [r4, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

    pop {r4,r5,r6,r7,pc} // popping pc is safe for armv4t when not switching mode

.balign 4

readRequestSettings:
    .word 0xA0406000

regBase:
    .word 0x04000198

cardDataRegAddress:
    .word 0x04100010

.global dspico_readSdSectorDma_miiCardDmaCopy32Ptr
dspico_readSdSectorDma_miiCardDmaCopy32Ptr:
    .word 0

.global dspico_readSdSectorDma_pollSdDataReadyPtr
dspico_readSdSectorDma_pollSdDataReadyPtr:
    .word 0

.pool

.section "dspico_readsdsectordma_pollSdDataReady", "ax"
.thumb
.global dspico_readSdSectorDma_pollSdDataReady
.type dspico_readSdSectorDma_pollSdDataReady, %function
dspico_readSdSectorDma_pollSdDataReady:
    ldrb r3, [r4, #0xF]
    lsrs r3, r3, #8
    bcs dspico_readSdSectorDma_pollSdDataReady
    // r3 is zero here

    // card_romSetCmd(0xE400000000000000ull);
    str r3, [r4, #0x14] // 00 00 00 00
    movs r3, #0xE4
    str r3, [r4, #0x10] // E4 00 00 00

    // card_romStartXfer
    ldr r3,= 0xA7446000
    str r3, [r4, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

1:
    ldrb r3, [r4, #0xE]
    lsrs r3, r3, #8
    bcc 1b

    ldr r3, [r5] // [0x04100010]
    cmp r3, #0
    beq dspico_readSdSectorDma_pollSdDataReady
    bx lr

.global dspico_finishReadSdSectorDma
.type dspico_finishReadSdSectorDma, %function
dspico_finishReadSdSectorDma:
    push {r4,r5,lr}
    ldr r4,= 0x04000198
    ldr r5,= 0x04100010

    movs r3, #0x80
    strb r3, [r4, #0x9] // select rom mode, without irq

    bl dspico_readSdSectorDma_pollSdDataReady
    pop {r4,r5,pc}

.balign 4

.pool
.end
