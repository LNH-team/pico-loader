.cpu arm7tdmi
.syntax unified
.section "r4idsn_writesd", "ax"
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global r4idsn_writeSd
.type r4idsn_writeSd, %function
r4idsn_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
.global r4idsn_writeSd_sdsc_shift
r4idsn_writeSd_sdsc_shift:
    lsls r7, r0, #9

    push {r0-r1}
    // SDIO cmd
    ldr r0, =0x180005AB
    // param
    movs r1, r7
    ldr r6, r4idsn_writeSd_sendSdioCommand_address
    bl blx_r6
    pop {r0-r1}

    // NOW we can start writing things
    movs r7, #1
    lsls r7, r7, #9
r4idsn_writeSd_block_write_loop:
    // start copy to MCCMD1
    ldmia r1!, {r5, r6}
    str r5, [r4, #0x8]
    str r6, [r4, #0xc]

    ldr r3, =0xA0406000
    str r3, [r4,#4]

r4idsn_writeSd_block_write_wait_busy:
    ldr r3, [r4,#4]
    cmp r3, #0
    blt r4idsn_writeSd_block_write_wait_busy

    subs r7, r7, #8
    bne r4idsn_writeSd_block_write_loop

    push {r0-r1}
    // Wait for SD state
    movs r0, #4
    movs r1, #0
    ldr r6, r4idsn_writeSd_sdWaitForState_address
    bl blx_r6
    pop {r0-r1}

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

blx_r6:
    bx r6

.balign 4

.global r4idsn_writeSd_sendSdioCommand_address
r4idsn_writeSd_sendSdioCommand_address:
    .word 0

.global r4idsn_writeSd_sdWaitForState_address
r4idsn_writeSd_sdWaitForState_address:
    .word 0

.pool

.end
