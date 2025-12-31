.cpu arm7tdmi
.syntax unified
.section "sccf_change_mode", "ax"

.global sccf_lockUnlockCard
.type sccf_lockUnlockCard, %function
sccf_lockUnlockCard:
	cmp r0, #1
	bne unlock
@ void sc_change_mode(uint16_t mode);
sccf_changeMode:
    ldr r2,= 0x09FFFFFE
    ldr r3,= 0xA55A
    strh r3, [r2]
    strh r3, [r2]
    strh r0, [r2]
    strh r0, [r2]
    mov pc, lr

unlock:
	movs r0, #3
	b sccf_changeMode

.pool
