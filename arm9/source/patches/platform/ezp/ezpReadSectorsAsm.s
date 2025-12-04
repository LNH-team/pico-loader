.cpu arm7tdmi
.section "ezp_readsectors", "ax"
.syntax unified
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global ezp_readSectors
.type ezp_readSectors, %function
ezp_readSectors:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

sector_loop:
    // request sd read for sector 0xaabbccdd
    // for xx number of sectors (up to 4 sectors)
    // B9 00 00 xx aa bb cc dd
    movs r3, #0xB9
    str r3, [r4,#0x8]

    // Maximum 4 sectors at a time.
    // Check if 3 or lower first.
    movs r7, #3
    ands r7, r7, r2
    bne sector_loop_skip_0_case // if between 1-3, then we can use value as is
    movs r7, #4 // if AND returns 0, then it's a 4 sector read
sector_loop_skip_0_case:
    strb r7, [r4, #0xB]

    lsrs r3, r0, #24
    strb r3, [r4,#0xC]
    lsrs r3, r0, #16
    strb r3, [r4,#0xD]
    lsrs r3, r0, #8
    strb r3, [r4,#0xE]
    strb r0, [r4,#0xF]

    ldr r6, =0x04100010

B9_poll_loop:
    ldr r3, =0xA75860C8
    str r3, [r4,#4]

B9_poll_transfer_loop:
    ldrb r3, [r4,#6]
    lsrs r3, r3, #8
    bcc B9_poll_check_transfer_end
    ldr r5, [r6]

B9_poll_check_transfer_end:
    ldrb r3, [r4,#7]
    lsrs r3, r3, #8
    bcs B9_poll_transfer_loop

    cmp r5, #0
    bne B9_poll_loop

    movs r3, r7
    ldr r6, ezp_readSectors_readSdData_address
    bl blx_r6

    adds r0, r0, r7
    subs r2, r2, r7
    bne sector_loop

    pop {r4-r7,pc}

blx_r6:
    bx r6

.balign 4

.global ezp_readSectors_readSdData_address
ezp_readSectors_readSdData_address:
    .word 0

.pool

.end
