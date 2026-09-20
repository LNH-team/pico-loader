.cpu arm946e-s
.section "fsstartoverlayhook", "ax"
.syntax unified
.thumb

.global fsstartoverlayhook_entry
.type fsstartoverlayhook_entry, %function
fsstartoverlayhook_entry:
    movs r0, #0x4
    add lr, r0
    push {lr}
    ldr r0, fsstartoverlayhook_hookFuncAddress
1:
    blx r0
    cmp r0, #0
    bne 1b

    // r0=[r5,#0x4], r1=[r5,#0x8] (DC_FlushRange args)
    // r4=[r5,#0x10] (instruction overwritten by patch in some signatures)
    adds r3, r5, #4
    ldm r3!, {r0-r2, r4}
    ldr r2, fsstartoverlayhook_dcFlushRangeAddress
    blx r2

    pop {pc}

.balign 4

.global fsstartoverlayhook_dcFlushRangeAddress
fsstartoverlayhook_dcFlushRangeAddress:
    .word 0

.global fsstartoverlayhook_hookFuncAddress
fsstartoverlayhook_hookFuncAddress:
    .word 0

.pool
.end
