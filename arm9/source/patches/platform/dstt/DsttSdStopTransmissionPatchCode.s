.cpu arm7tdmi
.syntax unified
.thumb

.section "dstt_sdstoptransmission", "ax"
.global dstt_sdStopTransmission
.type dstt_sdStopTransmission, %function
dstt_sdStopTransmission:
    // send SD_CMD12_STOP_TRANSMISSION
    push {r3-r6,lr}

    ldr r3, =0xA7180000
    ldr r4, =0x040001A0

    // select rom mode, without irq
    // the caller may not have ensured this is set
    movs r6, #0x80
    strb r6, [r4,#1]

    // set sd host mode DSTT_MODE_SDCMD_RESPONSE_32BIT, SD_CMD12_STOP_TRANSMISSION
    // 51 00 00 00 00 0C 01 00
    movs r5, #0x51
    str r5, [r4,#0x8]
    ldr r5,= 0x00010C00
    str r5, [r4,#0xC]
    str r3, [r4,#0x4]

    movs r6, #0x41
    lsls r6, r6, #20 // r6 = 0x04100000

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

    pop {r3-r6,pc}

.balign 4

.pool

.end
