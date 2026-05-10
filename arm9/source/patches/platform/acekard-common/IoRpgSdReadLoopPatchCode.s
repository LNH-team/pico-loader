.cpu arm7tdmi
.syntax unified
.section "iorpg_sdreadloop", "ax"
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global iorpg_sdReadLoop
.type iorpg_sdReadLoop, %function
iorpg_sdReadLoop:
    push {r5,lr}

    movs r5, #0x41
    lsls r5, r5, #20 // r5 = 0x04100000

sector_loop:
    // NOW we can start reading things
    movs r3, #0xB7
    str r3, [r4,#0x8]
    // The full CMD is B7 00 00 00 00 13 00 00
    // If we use 0x1300 and str, then it goes into the expected place
    movs r3, #0x13
    lsls r3, r3, #8
    str r3, [r4,#0xC]

    // previously SdWaitForState set this to 0xA7406004
    // We need this to become 0xA1406004
    movs r3, #0xA1
    strb r3, [r4,#7]

iorpg_sdReadLoop_read_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if data is ready
    bcc iorpg_sdReadLoop_read_loop_check_transfer_end // if not skip reading

    ldr r3, [r5, #0x10]
    stmia r1!, {r3}

iorpg_sdReadLoop_read_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs iorpg_sdReadLoop_read_loop

    // Wait for SD state
    push {r0-r1}
    movs r0, #7
    ldr r3, iorpg_sdReadLoop_sdWaitForState_address
    bl blx_r3
    pop {r0-r1}

    subs r2, #1
    bne sector_loop

    pop {r5,pc}

blx_r3:
    bx r3

.balign 4

.global iorpg_sdReadLoop_sdWaitForState_address
iorpg_sdReadLoop_sdWaitForState_address:
    .word 0

.pool

.end
