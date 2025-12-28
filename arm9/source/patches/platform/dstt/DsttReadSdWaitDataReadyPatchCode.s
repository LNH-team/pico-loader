.cpu arm7tdmi
.section "dstt_readsd_waitdataready", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global dstt_readSd_waitDataReady
.type dstt_readSd_waitDataReady, %function
dstt_readSd_waitDataReady:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    ldr r5, =0xA7180000

    movs r6, #0x41
    lsls r6, r6, #20

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

    pop {r4-r7,pc}

.balign 4

.pool

.end
