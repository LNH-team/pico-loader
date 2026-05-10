.cpu arm7tdmi
.syntax unified
.section "iorpg_writesd", "ax"
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global iorpg_writeSd
.type iorpg_writeSd, %function
iorpg_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
.global iorpg_writeSd_sdsc_shift
iorpg_writeSd_sdsc_shift:
    lsls r7, r0, #9

    push {r0-r1}
    // SDIO cmd
    ldr r0, iorpg_writeSd_cmd24_command
    // param
    movs r1, r7
    ldr r6, iorpg_writeSd_sendSdioCommand_address
    bl blx_r6
    pop {r0-r1}

    // NOW we can start writing things
    movs r7, #1
    lsls r7, r7, #9
iorpg_writeSd_block_write_loop:
    // start copy to MCCMD1
    ldmia r1!, {r5, r6}
    str r5, [r4, #0x8]
    str r6, [r4, #0xc]

    ldr r3, =0xA0406000
    str r3, [r4,#4]

iorpg_writeSd_block_write_wait_busy:
    ldr r3, [r4,#4]
    cmp r3, #0
    blt iorpg_writeSd_block_write_wait_busy

    subs r7, r7, #8
    bne iorpg_writeSd_block_write_loop

    push {r0-r1}
    // Wait for SD state
    movs r0, #0
    ldr r6, iorpg_writeSd_sdWaitForState_address
    bl blx_r6
    pop {r0-r1}

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

blx_r6:
    bx r6

.balign 4

.global iorpg_writeSd_sendSdioCommand_address
iorpg_writeSd_sendSdioCommand_address:
    .word 0

.global iorpg_writeSd_sdWaitForState_address
iorpg_writeSd_sdWaitForState_address:
    .word 0

.global iorpg_writeSd_cmd24_command
iorpg_writeSd_cmd24_command:
    .word 0

.pool

.end
