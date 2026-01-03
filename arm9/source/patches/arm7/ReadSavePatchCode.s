.cpu arm7tdmi
.section "readsave", "ax"
.syntax unified
.thumb

// r1 = save src
// r2 = memory dst
// r3 = byte length
.global readsave_asm
.type readsave_asm, %function
readsave_asm:
    push {r4,r5,r6,r7,lr}
    movs r4, r3 // remaining bytes
    movs r5, r1
    movs r6, r2
1:
    movs r0, r5 // will be rounded down by function
    ldr r3, readsave_save_offset_to_sd_sector_asm_address
    bl blx_r3
    cmp r0, #0
    beq end // out of bounds

    ldr r1, readsave_tmpBufferPtr
    movs r2, #1 // single sector
    lsls r7, r2, #9 // 512
    ldr r3, readsave_sdread_asm_address
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
    ldr r1, readsave_tmpBufferPtr

3:
    ldrb r3, [r1, r2]
    strb r3, [r0, r2]
    adds r2, #1
    subs r7, #1
    bne 3b

    cmp r4, #0
    bne 1b

end:
    pop {r4,r5,r6,r7,pc}

blx_r3:
    bx r3

.balign 4

.global readsave_tmpBufferPtr
readsave_tmpBufferPtr:
    .word 0

.global readsave_save_offset_to_sd_sector_asm_address
readsave_save_offset_to_sd_sector_asm_address:
    .word 0

.global readsave_sdread_asm_address
readsave_sdread_asm_address:
    .word 0

.pool

.end