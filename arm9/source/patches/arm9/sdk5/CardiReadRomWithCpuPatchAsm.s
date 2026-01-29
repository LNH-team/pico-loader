.cpu arm946e-s
.section "patch_cardireadromwithcpu", "ax"
.syntax unified

.thumb
.global patch_cardireadromwithcpu_entry
.type patch_cardireadromwithcpu_entry, %function
patch_cardireadromwithcpu_entry:

.global patch_cardireadromwithcpu_return_offset
patch_cardireadromwithcpu_return_offset:
    movs r0, #0x38
    add lr, r0
    push {lr}
    ldr r3, __patch_cardireadromwithcpu_fix_cp15_asm_address
    blx r3

.global patch_cardireadromwithcpu_mov_src_to_r0
patch_cardireadromwithcpu_mov_src_to_r0:
    nop
    ldr r3, __patch_cardireadromwithcpu_rom_offset_to_sd_sector_asm_address
    blx r3

    movs r2, #1 // r2 = sector count
.global patch_cardireadromwithcpu_mov_dst_to_r1
patch_cardireadromwithcpu_mov_dst_to_r1:
    movs r1, r5 // r1 = dst

.global patch_cardireadromwithcpu_mov_actual_dst_to_r3
patch_cardireadromwithcpu_mov_actual_dst_to_r3:
    mov r3, r10
    cmp r3, r1
    bne do_read // if dst != actual dst the cache is in use

.global patch_cardireadromwithcpu_mov_left_to_read_to_r2
patch_cardireadromwithcpu_mov_left_to_read_to_r2:
    mov r2, r8
    lsrs r2, r2, #9 // sectors left to read
    cmp r2, lr
    ble 1f
    mov r2, lr // clip to cluster boundary
1:
    lsls r7, r2, #9

do_read:
    lsrs r3, r1, #24 // if dst address is invalid (close to zero), ignore the read
    beq ignore_read  // this is intended to fix reads to null pointers that would be ignored if done with DMA

    ldr r3, __patch_cardireadromwithcpu_sdread_asm_address
    blx r3
    // arm
    // r7 = number of bytes read
    // r8 = number of bytes left to read
    // r10 = actual dst
    // thumb
    // r7 = number of bytes read
    // r5 = number of bytes left to read
    // [sp + 0 + 4] = actual dst

ignore_read:
    pop {pc}

.balign 4

.global __patch_cardireadromwithcpu_fix_cp15_asm_address
__patch_cardireadromwithcpu_fix_cp15_asm_address:
    .word 0

.global __patch_cardireadromwithcpu_rom_offset_to_sd_sector_asm_address
__patch_cardireadromwithcpu_rom_offset_to_sd_sector_asm_address:
    .word 0

.global __patch_cardireadromwithcpu_sdread_asm_address
__patch_cardireadromwithcpu_sdread_asm_address:
    .word 0

.pool

.end