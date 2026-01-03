.cpu arm7tdmi
.section "dstt_readsd", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global dstt_readSd
.type dstt_readSd, %function
dstt_readSd:
    push {r4-r7,lr}

.global dstt_readSd_sdsc_shift
dstt_readSd_sdsc_shift:
    lsls r7, r0, #9

    adr r0, dstt_stopTransmission_address
    ldmia r0, {r0, r3, r4}

    // request sd read for sector 0xaabbccdd
    // 54 aa bb cc dd 00 00 00
    movs r5, #0x54
    strb r5, [r4,#0x8]
    lsrs r5, r7, #24
    strb r5, [r4,#0x9]
    lsrs r5, r7, #16
    strb r5, [r4,#0xA]
    lsrs r5, r7, #8
    strb r5, [r4,#0xB]
    // make sure the last 3 bytes are zero
    movs r5, #0xFF
    ands r7, r5
    str r7, [r4,#0xC] // storing as little-endian puts the bottom 8 bits as first byte

    movs r7, #0x80
    strb r7, [r4,#1]

    movs r6, #0x41
    lsls r6, r6, #20

sector_loop:
    str r3, [r4,#4]

cmd54_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd54_wait_loop

    ldr r5, [r6, #0x10]
    // sd fifo full poll
    // 80 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    strb r7, [r4,#0x8]

cmd80_poll_loop:
    str r3, [r4,#4]

cmd80_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd80_wait_loop
    ldr r5, [r6, #0x10]
    cmp r5, #0
    bne cmd80_poll_loop

    // read sd fifo
    // 81 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x81
    strb r5, [r4,#0x8]
    movs r5, #0xA1
    strb r5, [r4,#7]

cmd81_data_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8 // check if data is ready
    bcc cmd81_data_loop_check_transfer_end // if not skip reading

    ldr r5, [r6, #0x10]
    stmia r1!, {r5}

cmd81_data_loop_check_transfer_end:
    ldrb r5, [r4,#7]
    lsrs r5, r5, #8 // check if transfer is done
    bcs cmd81_data_loop

    subs r2, #1
    beq stop_transmission

    // set sd host mode DSTT_MODE_RECEIVE_DATA_BLOCK_STOP_CLOCK
    // 51 00 00 00 00 00 07 00
    movs r5, #0x51
    str r5, [r4,#0x8]
    movs r5, #7
    lsls r5, r5, #16
    str r5, [r4,#0xC]

    b sector_loop

stop_transmission:
    bx r0

.balign 4

.global dstt_stopTransmission_address
dstt_stopTransmission_address:
    .word 0

settings:
    .word 0xA7180000

regbase:
    .word 0x040001A0

.pool

.section "dstt_readsd_stopTransmission", "ax"
.thumb
.global dstt_stopTransmission
.type dstt_stopTransmission, %function
dstt_stopTransmission:
    // send SD_CMD12_STOP_TRANSMISSION

    // set sd host mode DSTT_MODE_SDCMD_RESPONSE_32BIT, SD_CMD12_STOP_TRANSMISSION
    // 51 00 00 00 00 0C 01 00
    movs r5, #0x51
    str r5, [r4,#0x8]
    ldr r5,= 0x00010C00
    str r5, [r4,#0xC]
    str r3, [r4,#4]

cmd51_1_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd51_1_wait_loop
    ldr r5, [r6, #0x10]

    // sd busy poll
    // 50 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x50
    strb r5, [r4,#0x8]

cmd50_poll_loop:
    str r3, [r4,#4]

cmd50_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd50_wait_loop
    ldr r5, [r6, #0x10]
    cmp r5, #0
    bne cmd50_poll_loop

    // read sd response
    // 52 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x52
    strb r5, [r4,#0x8]
    str r3, [r4,#4]

cmd52_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd52_wait_loop
    ldr r5, [r6, #0x10]

    pop {r4-r7,pc}

.balign 4

.pool

.end