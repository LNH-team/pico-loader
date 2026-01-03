.cpu arm7tdmi
.section "melonds_readsd", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global melonds_readSd
.type melonds_readSd, %function
melonds_readSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // sd read for sector 0xaabbccdd
    // C0 aa bb cc dd 00 00 00
    movs r3, #0xC0
    strb r3, [r4,#0x8]
    lsrs r3, r0, #24
    strb r3, [r4,#0x9]
    lsrs r3, r0, #16
    strb r3, [r4,#0xA]
    lsrs r3, r0, #8
    strb r3, [r4,#0xB]
    lsls r7, r0, #24
    lsrs r7, r7, #24 // r7 = dd
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    ldr r6, =0x04100010
    ldr r3, =0xA1586000
    str r3, [r4,#4]

C0_data_loop:
    ldr r3, [r4, #4]
    lsrs r3, r3, #24 // check if data is ready
    bcc C0_data_loop_check_transfer_end // if not skip reading

    ldr r3, [r6]
    stmia r1!, {r3}

C0_data_loop_check_transfer_end:
    ldr r3, [r4, #4]
    lsrs r3, r3, #32 // check if transfer is done
    bcs C0_data_loop

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end
