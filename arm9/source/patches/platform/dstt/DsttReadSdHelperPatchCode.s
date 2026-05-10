.cpu arm7tdmi
.section "dstt_readsd_helper", "ax"
.syntax unified
.thumb

// r0 = src sector
.global dstt_readSd_applySectorCommand
.type dstt_readSd_applySectorCommand, %function
dstt_readSd_applySectorCommand:
    push {r5-r6,lr}

.global dstt_readSd_sdsc_shift
dstt_readSd_sdsc_shift:
    lsls r6, r0, #9

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

    pop {r5-r6,pc}

.balign 4

.global dstt_readSd_waitDataReady
.type dstt_readSd_waitDataReady, %function
dstt_readSd_waitDataReady:
    push {r5-r7,lr}

    movs r6, #0x41
    lsls r6, r6, #20 // r6 = 0x04100000

    // sd fifo full poll
    // 80 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r7, #0x80
    strb r7, [r4,#0x8]

    ldr r7, =0xA7180000
cmd80_poll_loop:
    str r7, [r4,#4]

cmd80_wait_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc cmd80_wait_loop
    ldr r5, [r6, #0x10]
    cmp r5, #0
    bne cmd80_poll_loop

    pop {r5-r7,pc}

.balign 4

// r1 = dst, will be incremented by 512
.global dstt_readSd_transferData
.type dstt_readSd_transferData, %function
dstt_readSd_transferData:
    push {r5-r7,lr}

    ldr r5, =0xA7180000

    movs r6, #0x41
    lsls r6, r6, #20 // r6 = 0x04100000

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

    pop {r5-r7,pc}

.balign 4

.pool

.end
