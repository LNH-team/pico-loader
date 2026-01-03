.cpu arm7tdmi
.syntax unified
.section "akrpg_writesd", "ax"
.thumb

// r0 = dst sector
// r1 = src
// r2 = sector count
.global akrpg_writeSd
.type akrpg_writeSd, %function
akrpg_writeSd:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
.global akrpg_writeSd_sdsc_shift
akrpg_writeSd_sdsc_shift:
    lsls r7, r0, #9

    push {r0-r1}
    // SDIO cmd
    ldr r0, =0x180005D5
    // param
    movs r1, r7
    ldr r6, akrpg_writeSd_sendSdioCommand_address
    bl blx_r6
    pop {r0-r1}

    // NOW we can start writing things
    movs r7, #1
    lsls r7, r7, #9
akrpg_writeSd_block_write_loop:
    // start copy to MCCMD1
    ldmia r1!, {r5, r6}
    str r5, [r4, #0x8]
    str r6, [r4, #0xc]

    ldr r3, =0xA0406000
    str r3, [r4,#4]

akrpg_writeSd_block_write_wait_busy:
    ldr r3, [r4,#4]
    cmp r3, #0
    blt akrpg_writeSd_block_write_wait_busy

    subs r7, r7, #8
    bne akrpg_writeSd_block_write_loop

    push {r0-r1}
    // Wait for SD state
    movs r0, #0
    movs r1, #0
    ldr r6, akrpg_writeSd_sdWaitForState_address
    bl blx_r6
    pop {r0-r1}

    adds r0, #1
    subs r2, #1
    bne sector_loop

    pop {r4-r7,pc}

blx_r6:
    bx r6

.balign 4

.global akrpg_writeSd_sendSdioCommand_address
akrpg_writeSd_sendSdioCommand_address:
    .word 0

.global akrpg_writeSd_sdWaitForState_address
akrpg_writeSd_sdWaitForState_address:
    .word 0

.pool

.end
