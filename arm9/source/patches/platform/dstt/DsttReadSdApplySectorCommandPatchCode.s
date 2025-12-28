.cpu arm7tdmi
.section "dstt_readsd_applysectorcommand", "ax"
.syntax unified
.thumb

// r0 = src sector
.global dstt_readSd_applySectorCommand
.type dstt_readSd_applySectorCommand, %function
dstt_readSd_applySectorCommand:
    push {r4-r6,lr}

.global dstt_readSd_sdsc_shift
dstt_readSd_sdsc_shift:
    lsls r6, r0, #9

    ldr r4, =0x040001A0

    // request sd read for sector 0xaabbccdd
    // 54 aa bb cc dd 00 00 00
    movs r5, #0x54
    strb r5, [r4,#0x8]
    lsrs r5, r6, #24
    strb r5, [r4,#0x9]
    lsrs r5, r6, #16
    strb r5, [r4,#0xA]
    lsrs r5, r6, #8
    strb r5, [r4,#0xB]
    // make sure the last 3 bytes are zero
    movs r5, #0xFF
    ands r6, r5
    str r6, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    pop {r4-r6,pc}

.balign 4

.pool

.end
