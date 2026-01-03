.cpu arm7tdmi
.section "ace3ds_writesd", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global ace3ds_writeSd
.type ace3ds_writeSd, %function
ace3ds_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    movs r5, #0xBC // for calculating command checksum
    // write at sd sector 0xaabbccdd
    // BB aa bb cc dd 00 00 00
    movs r3, #0xBB
    strb r3, [r4,#0x8]
    lsrs r3, r0, #24
    eors r5, r3
    strb r3, [r4,#0x9]
    lsrs r3, r0, #16
    eors r5, r3
    strb r3, [r4,#0xA]
    lsrs r3, r0, #8
    eors r5, r3
    strb r3, [r4,#0xB]
    lsls r7, r0, #24
    lsrs r7, r7, #24 // r7 = dd
    eors r5, r7
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    ldr r6, =0x04100010

    ldr r3, =0xE1586000
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
    strb r5, [r4,#0xF] // checksum

BC_poll_loop:
    ldr r3, =0xA7586000
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