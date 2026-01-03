.cpu arm7tdmi
.section "r4_readsd", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global r4_readSd
.type r4_readSd, %function
r4_readSd:
    push {r4-r7,lr}
    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // request sd read at byte address 0xaabbccdd
    // B9 aa bb cc dd 00 00 00
    lsls r7, r0, #9 // sd sector to sd byte address
    movs r3, #0xB9
    strb r3, [r4,#0x8]
    lsrs r3, r7, #24
    strb r3, [r4,#0x9]
    lsrs r3, r7, #16
    strb r3, [r4,#0xA]
    lsrs r3, r7, #8
    strb r3, [r4,#0xB]
    lsls r3, r7, #24
    lsrs r3, r3, #24 // r7 = dd
    str r3, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    ldr r6, =0x04100010

B9_poll_loop:
    ldr r3, =0xA7586000
    str r3, [r4,#4]

B9_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc B9_poll_check_transfer_end
    ldr r5, [r6]

B9_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs B9_poll_transfer_loop

    cmp r5, #0
    bne B9_poll_loop

    movs r3, #0xBA
    str r3, [r4,#0x8]
    str r5, [r4,#0xC] // r5 is 0 here

    ldr r3, =0xA1586000
    str r3, [r4,#4]

BA_data_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if data is ready
    bcc BA_data_loop_check_transfer_end // if not skip reading

    ldr r3, [r6]
    stmia r1!, {r3}

BA_data_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs BA_data_loop

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end