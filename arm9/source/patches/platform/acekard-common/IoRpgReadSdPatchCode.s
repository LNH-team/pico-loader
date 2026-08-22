.cpu arm7tdmi
.syntax unified
.section "iorpg_readsd", "ax"
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global iorpg_readSd
.type iorpg_readSd, %function
iorpg_readSd:
    push {r4,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

    push {r0-r1}

.global iorpg_readSd_sdsc_shift
iorpg_readSd_sdsc_shift:
    lsls r1, r0, #9

    ldr r0, iorpg_readSd_cmd18_command
    ldr r3, iorpg_readSd_sendSdioCommand_address
    bl blx_r3

    ldr r3, iorpg_readSd_cardWaitReady_address
    bl blx_r3
    pop {r0-r1}

    // Read sectors. Parameters identical to readSd
    ldr r3, iorpg_readSd_sdReadLoop_address
    bl blx_r3

    // Send CMD12 == STOP_TRANSMISSION
    push {r0-r1}
    ldr r0, iorpg_readSd_cmd12_command
    movs r1, #0
    ldr r3, iorpg_readSd_sendSdioCommand_address
    bl blx_r3

    pop {r0-r1,r4,pc}

blx_r3:
    bx r3

.balign 4

.global iorpg_readSd_sendSdioCommand_address
iorpg_readSd_sendSdioCommand_address:
    .word 0

.global iorpg_readSd_sdReadLoop_address
iorpg_readSd_sdReadLoop_address:
    .word 0

.global iorpg_readSd_cardWaitReady_address
iorpg_readSd_cardWaitReady_address:
    .word 0

.global iorpg_readSd_cmd18_command
iorpg_readSd_cmd18_command:
    .word 0

.global iorpg_readSd_cmd12_command
iorpg_readSd_cmd12_command:
    .word 0

.pool

.end
