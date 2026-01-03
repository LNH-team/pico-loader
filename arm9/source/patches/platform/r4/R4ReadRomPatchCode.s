.cpu arm7tdmi
.section "r4_readrom", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
.global r4_readRom
.type r4_readRom, %function
r4_readRom:
    push {r4-r7,lr}
    lsls r0, r0, #9 // rom sector to rom byte address
    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

    // request rom read at rom address 0xaabbccdd
    // B6 aa bb cc dd 00 00 00
    movs r3, #0xB6
    strb r3, [r4,#0x8]
    lsrs r3, r0, #24
    strb r3, [r4,#0x9]
    lsrs r3, r0, #16
    strb r3, [r4,#0xA]
    lsrs r3, r0, #8
    strb r3, [r4,#0xB]
    lsls r0, r0, #24
    lsrs r7, r0, #24 // r7 = dd
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    ldr r6, =0x04100010

B6_poll_loop:
    ldr r3, =0xA7586000
    str r3, [r4,#4]

B6_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc B6_poll_check_transfer_end
    ldr r2, [r6]

B6_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs B6_poll_transfer_loop

    cmp r2, #0
    bne B6_poll_loop

    movs r3, #0xB7
    strb r3, [r4,#0x8]

    ldr r3, =0xA1586000
    str r3, [r4,#4]

B7_data_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if data is ready
    bcc B7_data_loop_check_transfer_end // if not skip reading
    
    ldr r3, [r6]
    stmia r1!, {r3}

B7_data_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs B7_data_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end