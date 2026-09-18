.cpu arm946e-s
.syntax unified

.equ REG_TM3CNT_L, 0x0400010C
.equ OS_IE_TIMER3, 0x40

// The replacement and its timer interrupt handler are separate sections so that
// each fits in the small blocks of free space available in SDK 5 games.

.section "patch_cardireadromwithdma", "ax"

// Replacement for CARDi_ReadRomWithDma.
// r0 = request (+0x0C rom offset, +0x10 destination, +0x14 length)
//
// The data is read immediately through CARDi_ReadRomWithCPU, which is
// redirected by the card read patch. Completion is then signalled from a
// timer 3 interrupt rather than from this function, because callers wait for
// the completion callback to arrive in interrupt context and may start the
// next read from inside it.
.thumb
.global patch_cardireadromwithdma_entry
.type patch_cardireadromwithdma_entry, %function
patch_cardireadromwithdma_entry:
    push {r4, lr}
    ldr r1, patch_cardireadromwithdma_in_flight_slot
    str r0, [r1]

    ldr r1, [r0, #0x10] // destination
    ldr r2, [r0, #0x0C] // rom offset
    ldr r3, [r0, #0x14] // length
    ldr r4, patch_cardireadromwithdma_read_rom_with_cpu
    blx r4

    // The interrupt mask is left enabled after the first transfer. Masking it
    // again in the handler would race with a read started from inside the
    // completion callback, which re-enables it before the handler returns.
    movs r4, #OS_IE_TIMER3
    movs r0, r4
    ldr r1, patch_cardireadromwithdma_timer_irq_address
    ldr r3, patch_cardireadromwithdma_os_set_irq_function
    blx r3
    movs r0, r4
    ldr r3, patch_cardireadromwithdma_os_reset_request_irq_mask
    blx r3
    movs r0, r4
    ldr r3, patch_cardireadromwithdma_os_enable_irq_mask
    blx r3

    // Fire after about 120 microseconds (reload 0xFFC0, prescaler 64). The
    // delay lets the caller record that the read is pending before the
    // completion arrives.
    ldr r0,= REG_TM3CNT_L
    movs r1, #0x3F
    mvns r1, r1
    strh r1, [r0]
    movs r1, #0xC1
    strh r1, [r0, #2]
    pop {r4, pc}

.balign 4

.global patch_cardireadromwithdma_in_flight_slot
patch_cardireadromwithdma_in_flight_slot:
    .word 0

.global patch_cardireadromwithdma_read_rom_with_cpu
patch_cardireadromwithdma_read_rom_with_cpu:
    .word 0

.global patch_cardireadromwithdma_timer_irq_address
patch_cardireadromwithdma_timer_irq_address:
    .word 0

.global patch_cardireadromwithdma_os_set_irq_function
patch_cardireadromwithdma_os_set_irq_function:
    .word 0

.global patch_cardireadromwithdma_os_reset_request_irq_mask
patch_cardireadromwithdma_os_reset_request_irq_mask:
    .word 0

.global patch_cardireadromwithdma_os_enable_irq_mask
patch_cardireadromwithdma_os_enable_irq_mask:
    .word 0

.pool

.section "patch_cardireadromwithdma_irq", "ax"

// Timer 3 interrupt handler. Stops the timer and enters the completion tail of
// the SDK card DMA interrupt handler, which clears the in-flight request,
// restores the card state and invokes the request callback. The tail expects
// r4 = request and, for the larger handler variant, r5 = card DMA state, and
// it returns by popping the frame pushed on entry to that handler.
.thumb
.global patch_cardireadromwithdma_timer_irq
.type patch_cardireadromwithdma_timer_irq, %function
patch_cardireadromwithdma_timer_irq:
    ldr r0,= REG_TM3CNT_L
    movs r1, #0
    strh r1, [r0, #2]

    ldr r0, patch_cardireadromwithdma_irq_in_flight_slot
    ldr r0, [r0]
    cmp r0, #0
    beq 2f

    ldr r3, patch_cardireadromwithdma_irq_completion_tail
    ldr r2, patch_cardireadromwithdma_irq_state
    ldr r1, patch_cardireadromwithdma_irq_large_frame
    cmp r1, #0
    beq 1f
    push {r4, r5, r6, lr}
    movs r5, r2
    movs r4, r0
    bx r3
1:
    push {r4, lr}
    movs r4, r0
    bx r3
2:
    bx lr

.balign 4

.global patch_cardireadromwithdma_irq_in_flight_slot
patch_cardireadromwithdma_irq_in_flight_slot:
    .word 0

.global patch_cardireadromwithdma_irq_state
patch_cardireadromwithdma_irq_state:
    .word 0

.global patch_cardireadromwithdma_irq_large_frame
patch_cardireadromwithdma_irq_large_frame:
    .word 0

.global patch_cardireadromwithdma_irq_completion_tail
patch_cardireadromwithdma_irq_completion_tail:
    .word 0

.pool
.end
