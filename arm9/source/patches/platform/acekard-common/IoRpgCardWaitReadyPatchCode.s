.cpu arm7tdmi
.syntax unified
.section "iorpg_cardwaitready", "ax"
.thumb

.global iorpg_cardWaitReady
.type iorpg_cardWaitReady, %function
iorpg_cardWaitReady:
    push {r5-r7,lr}

    ldr r5, =0x04100010

    // Reset cmd buffer
    ldr r6, =0xB8B8B8B8
    str r6, [r4,#0x8]
    str r6, [r4,#0xC]

iorpg_cardWaitReady_read_loop:
    ldr r6, =0xA7406004
    str r6, [r4, #0x4]

iorpg_cardWaitReady_read_wait_data_ready:
    // Check MCCNT1_DATA_READY
    ldrb r7, [r4,#6]
    lsrs r7, r7, #8 // check if data is ready
    bcc iorpg_cardWaitReady_read_wait_data_ready // if not, loop

    ldr r7, [r5]
    ldr r6, =0xFC2 // card ID
    cmp r7, r6
    bne iorpg_cardWaitReady_read_loop

    pop {r5-r7,pc}

.balign 4

.pool

.end
