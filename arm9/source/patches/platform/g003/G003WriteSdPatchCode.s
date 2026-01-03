.cpu arm7tdmi
.section "g003_writesd", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global g003_writeSd
.type g003_writeSd, %function
g003_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // write at sd sector, sector << 1 == 0xaabbccdd
    // C5 bb cc dd 00 00 00 aa

    // left shift sector address 1
    lsls r7, r0, #1

    // start copy to MCCMD1
    movs r3, #0xC5
    strb r3, [r4,#0x8]
    lsrs r3, r7, #24
    strb r3, [r4,#0xF] // place MSByte of sector address at end of command instead
    lsrs r3, r7, #16
    strb r3, [r4,#0x9]
    lsrs r3, r7, #8
    strb r3, [r4,#0xA]
    // Ace3DS+ code uses str, but G003 doesn't store the last byte at u8 MCCMD1[4].
    // So we use strb just like the other bytes.
    strb r7, [r4,#0xB]

    ldr r6, =0x04100010

    ldr r3, =0xE1586000
    str r3, [r4,#4]

C5_data_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if ready to write
    bcc C5_data_loop_check_transfer_end // if not skip reading
    
    ldmia r1!, {r3}
    str r3, [r6]

C5_data_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs C5_data_loop

    movs r3, #0xC6
    strb r3, [r4,#0x8]

C6_poll_loop:
    ldr r3, =0xA7586000
    str r3, [r4,#4]

C6_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc C6_poll_check_transfer_end
    ldr r5, [r6]

C6_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs C6_poll_transfer_loop

    cmp r5, #0
    bne C6_poll_loop

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end
