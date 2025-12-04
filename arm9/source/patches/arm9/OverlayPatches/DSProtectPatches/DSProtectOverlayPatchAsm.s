.cpu arm946e-s
.syntax unified
.section "dsprotectpatch", "ax"
.thumb

.global dsprotectpatch_entry
.type dsprotectpatch_entry, %function
dsprotectpatch_entry:
    push {r4-r7, lr}

    // r5 is overlay table entry-- load { ovy_id, ram_start }
    ldmia r5!, {r6, r7}

    // Bail if this is the wrong overlay
    ldr r4, dsprotectpatch_overlayId
    cmp r4, r6
    bne continue_to_next

    // Try to patch A1
    ldr r0, dsprotectpatch_offsetA1
    adds r0, r7
    bl try_apply_patch

    // Try to patch NotA1
    ldr r0, dsprotectpatch_offsetNotA1
    adds r0, r7
    bl try_apply_patch

continue_to_next:
    // Return next patch address
    ldr r0, dsprotectpatch_nextAddress
    pop {r4-r7, pc}

.local try_apply_patch
.type try_apply_patch, %function
try_apply_patch:
    // If the high bit is set, this is invalid and we skip it
    cmp r0, #0x0
    blt offset_invalid

    // Check what patch type to use
    ldr r1, dsprotectpatch_patchType
    cmp r1, #0x0
    beq load_literal

    // Load next word (+4)
    ldr r2, [r0, #0x4]
    b load_done

load_literal:
    // Load literal value
    ldr r2, dsprotectpatch_writeWord

load_done:
    // Write to patch location
    str r2, [r0]

offset_invalid:
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

.global dsprotectpatch_overlayId
dsprotectpatch_overlayId:
    .word 0

.global dsprotectpatch_nextAddress
dsprotectpatch_nextAddress:
    .word 0

.pool

.end
