.cpu arm7tdmi
.syntax unified
.section "iorpg_sdhelper", "ax"
.thumb

// r0 = first 4 bytes of card command, bswap32'd
// r1 = param
.global iorpg_sendSdioCommand
.type iorpg_sendSdioCommand, %function
iorpg_sendSdioCommand:
    push {r4-r7,lr}

    ldr r4, =0x040001A0

    // start copy to MCCMD1
    str r0, [r4,#0x8]
    // SDIO param
    // need to bswap32...
    lsrs r6, r1, #24
    strb r6, [r4,#0xC]
    lsrs r6, r1, #16
    strb r6, [r4,#0xD]
    lsrs r6, r1, #8
    strb r6, [r4,#0xE]
    strb r1, [r4,#0xF]

    // flag when read size 0
    // SDIO cmds should add some latency as well
    ldr r6, =0xA0406050
    str r6, [r4, #0x4]

// loop until card is ready
iorpg_sendSdioCommand_read_loop:
    // Check MCCNT1_ENABLE
    ldrb r5, [r4,#7]
    lsrs r5, r5, #8
    bcs iorpg_sendSdioCommand_read_loop

    pop {r4-r7,pc}

.balign 4

.global iorpg_sdWaitForState
.type iorpg_sdWaitForState, %function
// r0 = state to wait for
iorpg_sdWaitForState:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    ldr r5, =0x04100010

    // Reset cmd buffer
    movs r6, #0xC0
    str r6, [r4,#0x8] // storing as little-endian puts the bottom 8 bits as first byte
    movs r6, #0
    str r6, [r4,#0xC] // clear buffer

iorpg_sdWaitForState_read_loop:
    ldr r6, =0xA7406004
    str r6, [r4, #0x4]

iorpg_sdWaitForState_read_wait_data_ready:
    // Check MCCNT1_DATA_READY
    ldrb r7, [r4,#6]
    lsrs r7, r7, #8 // check if data is ready
    bcc iorpg_sdWaitForState_read_wait_data_ready // if not, loop

    ldr r7, [r5]
    ldr r6, =0xFC2 // card ID
    cmp r7, r6
    beq iorpg_sdWaitForState_read_loop

.global iorpg_sdWaitForState_shift
iorpg_sdWaitForState_shift:
    lsrs r7, r7, #0

    movs r6, #0xF
    ands r7, r7, r6
    cmp r0, r7
    bne iorpg_sdWaitForState_read_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end
