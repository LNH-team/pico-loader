.cpu arm7tdmi
.section "patch_dsisdredirect", "ax"
.syntax unified
.thumb

// r0 = drive number
// r1 = start sector
// r2 = source/destination buffer
// r3 = sector count
// [sp] = isReading (0 = write, otherwise read)
.global __patch_dsisdredirect_io
.type __patch_dsisdredirect_io, %function
__patch_dsisdredirect_io:
    push {r4,lr}
    movs r0, r1
    movs r1, r2
    movs r2, r3
    ldr r4, [sp, #8] // reading
    cmp r4, #0
    bne 1f

    ldr r3, __patch_dsisdredirect_io_writesd_asm_address
    bl blx_r3
    movs r0, #1 // success
    b return

1:
    ldr r3, __patch_dsisdredirect_io_readsd_asm_address
    bl blx_r3

    movs r0, #1 // success
return:
    pop {r4}
    pop {r3}
blx_r3:
    bx r3

// r0 = drive number
// r1 = command
// r2 = argument buffer
.global __patch_dsisdredirect_control
.type __patch_dsisdredirect_control, %function
__patch_dsisdredirect_control:
    push {lr}
    cmp r1, #1 // startup
    bne returnZero

    ldr r3, __patch_dsisdredirect_control_get_drive_struct_address
    bl blx_r3
    ldr r1,= 0x4B4
    movs r2, #0
    str r2, [r0, r1] // partition 0
    subs r1, r1, #4
    ldr r2, [r0, r1]
    movs r3, #0x83 // valid, partitioned, inserted
    orrs r2, r2, r3
    str r2, [r0, r1]

returnZero:
    movs r0, #0
    pop {r3}
    bx r3

.balign 4

.global __patch_dsisdredirect_io_readsd_asm_address
__patch_dsisdredirect_io_readsd_asm_address:
    .word 0

.global __patch_dsisdredirect_io_writesd_asm_address
__patch_dsisdredirect_io_writesd_asm_address:
    .word 0

.global __patch_dsisdredirect_control_get_drive_struct_address
__patch_dsisdredirect_control_get_drive_struct_address:
    .word 0

.pool

.end