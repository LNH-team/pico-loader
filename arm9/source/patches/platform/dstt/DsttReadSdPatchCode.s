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

    ldr r7, dstt_readSd_applySectorCommand_address
    bl blx_r7

    ldr r3, =0xA7180000
    ldr r4, =0x040001A0

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
    ldr r7, dstt_waitDataReady_address
    bl blx_r7

    ldr r7, dstt_transferData_address
    bl blx_r7

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
    ldr r7, dstt_stopTransmission_address
    bl blx_r7
    pop {r4-r7,pc}

blx_r7:
    bx r7

.balign 4

.global dstt_readSd_applySectorCommand_address
dstt_readSd_applySectorCommand_address:
    .word 0

.global dstt_waitDataReady_address
dstt_waitDataReady_address:
    .word 0

.global dstt_transferData_address
dstt_transferData_address:
    .word 0

.global dstt_stopTransmission_address
dstt_stopTransmission_address:
    .word 0

.pool

.end
