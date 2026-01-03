.cpu arm7tdmi
.section "verifysave", "ax"
.syntax unified
.thumb

// r1 = memory src
// r2 = save src
// r3 = byte length
.global verifysave_asm
.type verifysave_asm, %function
verifysave_asm:
    push {r4,r5,r6,r7,lr}
    movs r4, r3 // remaining bytes
    movs r5, r2
    movs r6, r1
1:
    movs r0, r5 // will be rounded down by function
    ldr r3, verifysave_save_offset_to_sd_sector_asm_address
    bl blx_r3
    cmp r0, #0
    beq end // out of bounds

    ldr r1, verifysave_tmpBufferPtr
    movs r2, #1 // single sector
    lsls r7, r2, #9 // 512
    ldr r3, verifysave_sdread_asm_address
    bl blx_r3

    // verify bytes
    lsls r2, r5, #23
    lsrs r2, r2, #23 // r2 = byte offset in sector
    subs r0, r6, r2
    subs r7, r7, r2 // remaining in sector
    cmp r7, r4 // if remaining in sector > requested read length
    bls 2f
    movs r7, r4 // clamp to requested read length
2:
    subs r4, r7
    adds r5, r7
    adds r6, r7
    ldr r1, verifysave_tmpBufferPtr

3:
    ldrb r3, [r1, r2]
    mov lr, r3
    ldrb r3, [r0, r2]
    cmp lr, r3
    bne bad_end
    adds r2, #1
    subs r7, #1
    bne 3b

    cmp r4, #0
    bne 1b

end:
    movs r0, #0
    pop {r4,r5,r6,r7,pc}

bad_end:
    movs r0, #1
    pop {r4,r5,r6,r7,pc}

blx_r3:
    bx r3

.balign 4

.global verifysave_tmpBufferPtr
verifysave_tmpBufferPtr:
    .word 0

.global verifysave_save_offset_to_sd_sector_asm_address
verifysave_save_offset_to_sd_sector_asm_address:
    .word 0

.global verifysave_sdread_asm_address
verifysave_sdread_asm_address:
    .word 0

.pool

.end