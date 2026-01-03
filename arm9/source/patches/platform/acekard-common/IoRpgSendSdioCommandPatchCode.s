.cpu arm7tdmi
.syntax unified
.section "iorpg_sendsdiocommand", "ax"
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

.pool

.end
