.cpu arm7tdmi
.syntax unified
.section "ak2_sdreadsector", "ax"
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global ak2_sdReadSector
.type ak2_sdReadSector, %function
ak2_sdReadSector:
    push {r4-r5,lr}

    ldr r4, =0x040001A0
    ldr r5, =0x04100010

sector_loop:
    // NOW we can start reading things
    movs r3, #0xB7
    str r3, [r4,#0x8]
    // The full CMD is B7 00 00 00 00 13 00 00
    // If we use 0x1300 and str, then it goes into the expected place
    movs r3, #0x13
    lsls r3, r3, #8
    str r3, [r4,#0xC]

    ldr r3, =0xA1406004
    str r3, [r4,#4]

ak2_sdReadSector_read_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8 // check if data is ready
    bcc ak2_sdReadSector_read_loop_check_transfer_end // if not skip reading

    ldr r3, [r5]
    stmia r1!, {r3}

ak2_sdReadSector_read_loop_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8 // check if transfer is done
    bcs ak2_sdReadSector_read_loop

    // Wait for SD state
    push {r0-r1}
    movs r0, #4
    movs r1, #7
    ldr r3, ak2_sdReadSector_sdWaitForState_address
    bl blx_r3
    pop {r0-r1}

    subs r2, #1
    bne sector_loop

    pop {r4-r5,pc}

blx_r3:
    bx r3

.balign 4

.global ak2_sdReadSector_sdWaitForState_address
ak2_sdReadSector_sdWaitForState_address:
    .word 0

.pool

.end
