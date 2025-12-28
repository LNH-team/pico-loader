.cpu arm7tdmi
.section "dstt_readsd_transferdata", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global dstt_readSd_transferData
.type dstt_readSd_transferData, %function
dstt_readSd_transferData:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    ldr r5, =0xA7180000

    movs r6, #0x41
    lsls r6, r6, #20

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

    pop {r4-r7,pc}

.balign 4

.pool

.end
