.cpu arm7tdmi
.section "melonds_writesd", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global melonds_writeSd
.type melonds_writeSd, %function
melonds_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // write at sd sector 0xaabbccdd
    // C1 aa bb cc dd 00 00 00
    movs r3, #0xC1
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
    ldr r3, =0xE1586000
    str r3, [r4,#4]

C1_data_loop:
    ldr r3, [r4, #4]
    lsrs r3, r3, #24 // check if ready to write
    bcc C1_data_loop_check_transfer_end // if not skip reading

    ldmia r1!, {r3}
    str r3, [r6]

C1_data_loop_check_transfer_end:
    ldr r3, [r4, #4]
    lsrs r3, r3, #32 // check if transfer is done
    bcs C1_data_loop

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end
