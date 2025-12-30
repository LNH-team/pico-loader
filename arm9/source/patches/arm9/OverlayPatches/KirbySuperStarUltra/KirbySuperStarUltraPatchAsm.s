.cpu arm946e-s
.syntax unified
.section "kirbyultrapatch", "ax"
.thumb

.global kirbyultrapatch_entry
.type kirbyultrapatch_entry, %function
kirbyultrapatch_entry:
    push {r4-r7, lr}
    ldm r5!, {r0, r1} // ovy_id, ram_start
    cmp r0, #6
    bne continue_to_next

    ldr r0, kirbyultrapatch_offset
    adds r1, r0
    ldm r1!, {r2, r3, r4, r5, r6, r7}
    subs r1, #24 // Reset pointer
    subs r2, #3 // Correct function call offset
    stm r1!, {r5, r6, r7} // Swap the two blocks of 3 instructions
    stm r1!, {r2, r3, r4}

continue_to_next:
    ldr r0, kirbyultrapatch_nextAddress
    pop {r4-r7, pc}

.balign 4

.global kirbyultrapatch_offset
kirbyultrapatch_offset:
    .word 0

.global kirbyultrapatch_nextAddress
kirbyultrapatch_nextAddress:
    .word 0

.pool

.end
