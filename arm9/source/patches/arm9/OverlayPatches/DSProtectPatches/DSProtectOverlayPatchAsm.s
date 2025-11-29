.cpu arm946e-s
.section "dsprotectpatch", "ax"
.syntax unified
.thumb

.global dsprotectpatch_entry
.type dsprotectpatch_entry, %function
dsprotectpatch_entry:
    push {r4-r7,lr}

    ldmia r5!, {r6,r7} // ovy_id, ram_start

    ldr r4, dsprotectpatch_overlay_id
    cmp r4, r6
    bne continue_to_next

    // 0xFFFFFFFF for invalid comparing
    movs r4, #0
    subs r4, r4, #1

    // Try to patch A1
    ldr r0, dsprotectpatch_offsetA1
    cmp r0, r4
    beq invalid_a1
    adds r0, r7
    bl applypatch

invalid_a1:
    // Try to patch NotA1
    ldr r0, dsprotectpatch_offsetNotA1
    cmp r0, r4
    beq invalid_not_a1
    adds r0, r7
    bl applypatch

invalid_not_a1:
continue_to_next:
    ldr r0, dsprotectpatch_nextAddress
    pop {r4-r7,pc}


.local applypatch
.type applypatch, %function
applypatch:
    ldr r1, dsprotectpatch_patchType
    cmp r1, 0
    beq load_literal
    ldr r2, [r0, #0x4]
    b load_done

load_literal:
    ldr r2, dsprotectpatch_writeWord

load_done:
    str r2, [r0]
    bx lr


.balign 4

.global dsprotectpatch_writeWord
dsprotectpatch_writeWord:
    .word 0

.global dsprotectpatch_patchType
dsprotectpatch_patchType:
    .word 0

.global dsprotectpatch_offsetA1
dsprotectpatch_offsetA1:
    .word 0

.global dsprotectpatch_offsetNotA1
dsprotectpatch_offsetNotA1:
    .word 0

.global dsprotectpatch_overlay_id
dsprotectpatch_overlay_id:
    .word 0

.global dsprotectpatch_nextAddress
dsprotectpatch_nextAddress:
    .word 0


.end
