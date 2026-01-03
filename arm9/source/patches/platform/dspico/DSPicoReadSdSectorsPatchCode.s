.cpu arm7tdmi
.section "dspico_readsdsectors", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global dspico_readSdSectors
.type dspico_readSdSectors, %function
dspico_readSdSectors:
    push {r0,r1,r2,r4-r7,lr}
    pop {r4,r5,r6}

1:
    // card_romSetCmd(0xE300000000000000ull | sector);
    adr r2, read_request_settings
    ldmia r2, {r1,r2}
    movs r0, #0xE3
    str r0, [r2, #0x10] // E3 00 00 00
    movs r0, r4
    rors r0, r2 // ror 24
    str r0, [r2, #0x14] // AA XX CC XX
    strb r4, [r2, #0x17] // AA XX CC DD
    lsrs r0, r4, #16
    strb r0, [r2, #0x15]

    // card_romStartXfer
    movs r0, #0x80
    strb r0, [r2, #0x9]
    str r1, [r2, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

    ldr r3, dspico_readsdsectors_readDirectAddress
    bl blx_r3

    pop {r4-r7,pc} // popping pc is safe for armv4t when not switching mode

blx_r3:
    bx r3

.balign 4

read_request_settings:
    .word 0xA0406000

reg_base:
    .word 0x04000198

.global dspico_readsdsectors_readDirectAddress
dspico_readsdsectors_readDirectAddress:
    .word 0

.pool

.section "dspico_readsdsectors_readdirect", "ax"
.thumb
.global dspico_readSdSectorsDirect
.type dspico_readSdSectorsDirect, %function
dspico_readSdSectorsDirect:
    push {r4,r6,r7,lr}
    b poll_read_done
1:
    movs r0, #0xE5
    str r0, [r2, #0x10] // E5 00 00 00
    movs r0, #0
    str r0, [r2, #0x14] // 00 00 00 00

    ldr r0,= 0xA1444000

    // card_romStartXfer
3:
    ldrb r4, [r2, #0xF]
    lsrs r4, r4, #8
    bcs 3b

    str r0, [r2, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

    movs r0, #128
    ldr r4,= 0x04100000
2:
    ldrb r3, [r2, #0xE]
    lsrs r3, r3, #8
    bcc 2b

    ldr r3, [r4, #0x10]
    stmia r5!, {r3}
    subs r0, #1
    bne 2b

    subs r6, #1 // count -= len

    // card_romWaitBusy
poll_read_done:
3:
    ldrb r1, [r2, #0xF]
    lsrs r1, r1, #8
    bcs 3b

    // card_romSetCmd(0xE400000000000000ull);
    movs r1, #0xE4
    str r1, [r2, #0x10] // E4 00 00 00
    movs r1, #0
    str r1, [r2, #0x14] // 00 00 00 00

    // card_romStartXfer
    ldr r1,= 0xA7446000
    str r1, [r2, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

4:
    ldrb r1, [r2, #0xE]
    lsrs r1, r1, #8
    bcc 4b

    movs r1, #0x41
    lsls r1, r1, #20 // 0x04100000
    ldr r1, [r1, #0x10]
    cmp r1, #0
    beq poll_read_done

    cmp r6, #0
    bne 1b

    pop {r4,r6,r7,pc}

.balign 4

.pool

.end