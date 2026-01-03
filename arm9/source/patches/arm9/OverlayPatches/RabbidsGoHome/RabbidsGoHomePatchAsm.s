.cpu arm946e-s
.syntax unified
.section "rabbidspatch", "ax"
.thumb

.global rabbidspatch_entry
.type rabbidspatch_entry, %function
rabbidspatch_entry:
    push {r5,lr}
    ldm r5!, {r0, r1} // ovy_id, ram_start
    cmp r0, #1
    bne continue_to_next

    ldr r2, rabbidspatch_offset
    ldr r0, =0xE3A00001  // mov r0, #1
    str r0, [r1, r2]

continue_to_next:
    ldr r0, rabbidspatch_nextAddress
    pop {r5,pc}

.balign 4

.global rabbidspatch_offset
rabbidspatch_offset:
    .word 0

.global rabbidspatch_nextAddress
rabbidspatch_nextAddress:
    .word 0

.pool

.end
