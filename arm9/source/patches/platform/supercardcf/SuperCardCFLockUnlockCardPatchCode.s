.cpu arm7tdmi
.syntax unified
.thumb
.section "sccf_lock_unlock", "ax"

.global sccf_lockUnlockCard
.type sccf_lockUnlockCard, %function
sccf_lockUnlockCard:
    cmp r0, #1
    bne unlock
@ void sc_change_mode(uint16_t mode);
sccf_changeMode:
    push {r1-r3, lr}
    ldr r2,= 0x09FFFFFE
    ldr r3,= 0xA55A
    strh r3, [r2]
    strh r3, [r2]
    strh r0, [r2]
    strh r0, [r2]
    pop {r1-r3, pc}

unlock:
    movs r0, #3
    b sccf_changeMode

.pool
