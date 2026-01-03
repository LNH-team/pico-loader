.cpu arm7tdmi
.section "dspico_writesdsectors", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global dspico_writeSdSectors
.type dspico_writeSdSectors, %function
dspico_writeSdSectors:
    push {r0,r1,r2,r4-r7,lr}
    pop {r4,r5,r6}

1:
    // card_romSetCmd(0xF6E10D9B00000000ull | sector);
    adr r2, write_cmd_rev
    ldmia r2, {r0,r1,r2,r3}
    str r0, [r2, #0x10] // F6 E1 0D 9B
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

    movs r1, #0x41
    lsls r1, r1, #20 // 0x04100000
2:
    ldrb r7, [r2, #0xE]
    lsrs r7, r7, #8
    bcc 2b

    ldmia r5!, {r7}
    str r7, [r1, #0x10]
    subs r0, #1
    bne 2b

    // card_romWaitBusy
poll_write_done:
3:
    ldrb r7, [r2, #0xF]
    lsrs r7, r7, #8
    bcs 3b

    // card_romSetCmd(0xE400000000000000ull);
    movs r7, #0xE4
    str r7, [r2, #0x10] // E4 00 00 00
    movs r7, #0
    str r7, [r2, #0x14] // 00 00 00 00

    // card_romStartXfer
    str r3, [r2, #0xC] // REG_MCCNT1 = MCCNT1_ENABLE | settings;

4:
    ldrb r7, [r2, #0xE]
    lsrs r7, r7, #8
    bcc 4b

    ldr r7, [r1, #0x10]
    cmp r7, #0
    beq poll_write_done

    adds r4, #1
    subs r6, #1
    bne 1b

    pop {r4-r7,pc} // popping pc is safe for armv4t when not switching mode

.balign 4

write_cmd_rev:
    .word 0x9B0DE1F6

write_cmd_settings:
    .word 0xE1486000

reg_base:
    .word 0x04000198

write_poll_settings:
    .word 0xA7446000

.pool

.end