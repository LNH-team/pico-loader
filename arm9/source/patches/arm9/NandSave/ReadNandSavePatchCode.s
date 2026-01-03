.cpu arm946e-s
.section "patch_readnandsave", "ax"
.syntax unified
.thumb

// r0 = memory dst
// r1 = nand byte offset
// r2 = byte length
// returns r0 = bool success
.global patch_readNandSave
.type patch_readNandSave, %function
patch_readNandSave:
    push {r0,r1,r2,r4,r5,r6,lr}
    pop {r4,r5,r6}
    lsrs r6, r6, #9 // remaining number of sectors = byte length / 512
loop:
    // while remaining number of sectors > 0
    cmp r6, #0
    beq end

    movs r0, r5 // nand byte offset
    ldr r3, patch_readNandSave_save_offset_to_sd_sector_asm_address
    blx r3

    mov r2, lr // sectors to read
    // if sectors to read > remaining number of sectors
    cmp r2, r6
    bls 1f
    movs r2, r6 // sectors to read = remaining number of sectors
1:
    subs r6, r2 // remaining number of sectors -= sectors to read
    movs r1, r4 // memory dst
    lsls r3, r2, #9
    adds r4, r3 // memory dst += sectors to read * 512
    adds r5, r3 // nand byte offset += sectors to read * 512

    ldr r3, patch_readNandSave_sdread_asm_address
    blx r3

    b loop

end:
    movs r0, #1
    pop {r4,r5,r6,pc}

.balign 4
.pool

.global patch_readNandSave_save_offset_to_sd_sector_asm_address
patch_readNandSave_save_offset_to_sd_sector_asm_address:
    .word 0

.global patch_readNandSave_sdread_asm_address
patch_readNandSave_sdread_asm_address:
    .word 0

.end
