.cpu arm7tdmi
.section "stargate_writesd", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global stargate_writeSd
.type stargate_writeSd, %function
stargate_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // write at sd sector 0xaabbccdd
    // BB aa bb cc dd 00 00 00
    movs r3, #0xBB
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

    ldr r3, =0xE1400000
    str r3, [r4,#4]

BB_data_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if ready to write
    bcc BB_data_loop_check_transfer_end // if not skip reading

    ldmia r1!, {r3}
    str r3, [r6]

BB_data_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs BB_data_loop

    movs r3, #0xBC
    strb r3, [r4,#0x8]

BC_poll_loop:
    ldr r3, =0xA7406000
    str r3, [r4,#4]

BC_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc BC_poll_check_transfer_end
    ldr r5, [r6]

BC_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs BC_poll_transfer_loop

    cmp r5, #0
    bne BC_poll_loop

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end