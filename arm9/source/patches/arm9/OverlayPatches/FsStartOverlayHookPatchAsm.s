.cpu arm946e-s
.section "fsstartoverlayhook", "ax"
.syntax unified
.thumb

.global fsstartoverlayhook_entry
.type fsstartoverlayhook_entry, %function
fsstartoverlayhook_entry:
    movs r0, #0x4
    add lr, r0
    ldr r2, fsstartoverlayhook_dcFlushRangeAddress
    push {r2,lr}
    ldr r0, fsstartoverlayhook_hookFuncAddress
1:
    blx r0
    cmp r0, #0
    bne 1b

    ldr r0, [r5, #4]
    ldr r1, [r5, #8]
    pop {r2}
    blx r2
    ldr r4, [r5, #0x10]
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
