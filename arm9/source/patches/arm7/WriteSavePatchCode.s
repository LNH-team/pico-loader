.cpu arm7tdmi
.section "writesave", "ax"
.syntax unified
.thumb

// r1 = memory src
// r2 = save dst
// r3 = byte length
.global writesave_asm
.type writesave_asm, %function
writesave_asm:
    push {r4,r5,r6,r7,lr}
    movs r4, r3 // remaining bytes
    movs r5, r2
    movs r6, r1
1:
    movs r0, r5 // will be rounded down by function
    ldr r3, writesave_save_offset_to_sd_sector_asm_address
    bl blx_r3
    cmp r0, #0
    beq end // out of bounds

    push {r0}

    ldr r1, writesave_tmpBufferPtr
    movs r2, #1 // single sector
    lsls r7, r2, #9 // 512
    ldr r3, writesave_sdread_asm_address
    bl blx_r3

    // copy bytes
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
    ldr r1, writesave_tmpBufferPtr

3:
    ldrb r3, [r0, r2]
    strb r3, [r1, r2]
    adds r2, #1
    subs r7, #1
    bne 3b

    pop {r0}
    movs r2, #1 // single sector
    ldr r3, writesave_sdwrite_asm_address
    bl blx_r3

    cmp r4, #0
    bne 1b

end:
    pop {r4,r5,r6,r7,pc}

blx_r3:
    bx r3

.balign 4

.global writesave_tmpBufferPtr
writesave_tmpBufferPtr:
    .word 0

.global writesave_save_offset_to_sd_sector_asm_address
writesave_save_offset_to_sd_sector_asm_address:
    .word 0

.global writesave_sdread_asm_address
writesave_sdread_asm_address:
    .word 0

.global writesave_sdwrite_asm_address
writesave_sdwrite_asm_address:
    .word 0

.pool

.end