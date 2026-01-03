.cpu arm7tdmi
.section "dstt_writesd", "ax"
.syntax unified
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global dstt_writeSd
.type dstt_writeSd, %function
dstt_writeSd:
    push {r4-r7,lr}

.global dstt_writeSd_sdsc_shift
dstt_writeSd_sdsc_shift:
    lsls r0, r0, #9

    adr r7, settings
    ldmia r7, {r3, r4, r5, r6, r7}

    mov lr, r7
    movs r7, #0x80
    strb r7, [r4,#1]
    mov r7, lr

    // send SD_CMD25_WRITE_MULTIPLE_BLOCK for sector 0xaabbccdd

    // set sd host mode DSTT_MODE_SDCMD_RESPONSE_32BIT, SD_CMD25_WRITE_MULTIPLE_BLOCK
    // 51 aa bb cc dd 19 01 00
    str r5, [r4,#0xC]
    movs r5, #0x51
    strb r5, [r4,#0x8]
    lsrs r5, r0, #24
    strb r5, [r4,#0x9]
    lsrs r5, r0, #16
    strb r5, [r4,#0xA]
    lsrs r5, r0, #8
    strb r5, [r4,#0xB]
    strb r0, [r4,#0xC]

    ldr r0, dstt_writeSdContinue_address
    bx r0

.balign 4

.global dstt_writeSdContinue_address
dstt_writeSdContinue_address:
    .word 0

settings:
    .word 0xA7180000

regbase:
    .word 0x040001A0

writeStartCommandLo:
    .word 0x00011900

regbase2:
    .word 0x04100000

terminateCommandLo:
    .word 0x00010C00

.pool

.section "dstt_writesd_continue", "ax"
.thumb
.global dstt_writeSdContinue
.type dstt_writeSdContinue, %function
dstt_writeSdContinue:
    bl do32BitTransfer
    bl waitSdHostBusy
    bl readSdResponse

sector_loop:
    // write to fifo
    // 82 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x82
    strb r5, [r4,#0x8]
    movs r5, #0xE1
    strb r5, [r4,#7]

cmd82_data_loop:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8 // check if ready to write
    bcc cmd82_data_loop_check_transfer_end // if not skip writing

    ldmia r1!, {r5}
    str r5, [r6, #0x10]

cmd82_data_loop_check_transfer_end:
    ldrb r5, [r4,#7]
    lsrs r5, r5, #8 // check if transfer is done
    bcs cmd82_data_loop

    bl waitSdHostBusy

    subs r2, #1
    bne skip_terminate

terminate:
    // send SD_CMD12_STOP_TRANSMISSION

    // set sd host mode DSTT_MODE_SDCMD_RESPONSE_32BIT, SD_CMD12_STOP_TRANSMISSION
    // 51 00 00 00 00 0C 01 00
    str r7, [r4,#0xC]
    movs r5, #0x51
    bl do32BitTransferWriteCmdHi
    bl waitSdHostBusy
    bl readSdResponse

skip_terminate:
    // wait sddat[0] == 1
    // 56 00 00 00 00 00 00 00
    movs r5, #0x56
    bl do32BitTransferWriteCmdHi
    bl waitSdHostBusy
    cmp r2, #0
    bne sector_loop

    pop {r4-r7,pc}

waitSdHostBusy:
    mov r12, lr
cmd50_poll_loop:
    // sd busy poll
    // 50 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x50
    bl do32BitTransferWriteCmdHi
    cmp r5, #0
    bne cmd50_poll_loop
    bx r12

readSdResponse:
    // read sd response
    // 52 xx xx xx xx xx xx xx
    // xx bytes are leftover from previous command
    movs r5, #0x52
do32BitTransferWriteCmdHi:
    strb r5, [r4,#0x8]
do32BitTransfer:
    str r3, [r4,#4]
1:
    ldrb r5, [r4,#6]
    lsrs r5, r5, #8
    bcc 1b
    ldr r5, [r6, #0x10]
    bx lr

.balign 4

.pool

.end