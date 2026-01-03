.cpu arm7tdmi
.syntax unified
.section "iorpg_sdwaitforstate", "ax"
.thumb

.global iorpg_sdWaitForState
.type iorpg_sdWaitForState, %function
// r0 = state shift, some carts put it in different bytes
// r1 = state to wait for
iorpg_sdWaitForState:
    push {r4-r7,lr}

    ldr r4, =0x040001A0
    ldr r5, =0x04100010

    // Reset cmd buffer
    movs r6, #0xC0
    str r6, [r4,#0x8] // storing as little-endian puts the bottom 8 bits as first byte
    movs r6, #0
    str r6, [r4,#0xC] // clear buffer

iorpg_sdWaitForState_read_loop:
    ldr r6, =0xA7406004
    str r6, [r4, #0x4]

iorpg_sdWaitForState_read_wait_data_ready:
    // Check MCCNT1_DATA_READY
    ldrb r7, [r4,#6]
    lsrs r7, r7, #8 // check if data is ready
    bcc iorpg_sdWaitForState_read_wait_data_ready // if not, loop

    ldr r7, [r5]
    ldr r6, =0xFC2 // card ID
    cmp r7, r6
    beq iorpg_sdWaitForState_read_loop
    lsrs r7, r7, r0
    movs r6, #0xF
    ands r7, r7, r6
    cmp r1, r7
    bne iorpg_sdWaitForState_read_loop

    pop {r4-r7,pc}

.balign 4

.pool

.end
