.cpu arm7tdmi
.section "ezp_readsddata", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
// r3 = sector count for single read
.global ezp_readSdData
.type ezp_readSdData, %function
ezp_readSdData:
    push {r4-r7,lr}
    ldr r4, =0x040001A0

    // Setup command for data read
    // BA 00 00 00 00 00 00 00
    movs r7, #0xBA
    str r7, [r4,#0x8]
    movs r7, #0
    str r7, [r4,#0xC]

    // Set up MCCNT for required read size
    adr r6, readLengthControlValues
    lsls r7, r3, #2
    ldr r5, [r6,r7]
    str r5, [r4,#4]

    ldr r6, =0x04100010

    // Get number of words to read
    // 1 sector == 128 words
    lsls r3, r3, #7

BA_data_loop:
    ldrb r7, [r4,#6]
    lsrs r7, r7, #8 // check if data is ready
    bcc BA_data_loop_check_transfer_end // if not skip reading

    ldr r7, [r6]
    // Check if the read word needs to be stored.
    cmp r3, #0
    // If we already read all the needed words, skip
    beq BA_data_loop_check_transfer_end
    subs r3, r3, #1
    stmia r1!, {r7}

BA_data_loop_check_transfer_end:
    ldrb r7, [r4,#7]
    lsrs r7, r7, #8 // check if transfer is done
    bcs BA_data_loop

    pop {r4-r7,pc}

.balign 4

readLengthControlValues:
    .word 0
    .word 0xA15A6000
    .word 0xA25A6000
    .word 0xA35A6000
    .word 0xA35A6000

.pool

.end
