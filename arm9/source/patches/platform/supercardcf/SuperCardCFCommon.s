.cpu arm7tdmi
.syntax unified
.section "sccf_change_mode", "ax"

@ void sc_change_mode(uint16_t mode);
sccf_changeMode:
    ldr r2,= 0x09FFFFFE
    ldr r3,= 0xA55A
    strh r3, [r2]
    strh r3, [r2]
    strh r0, [r2]
    strh r0, [r2]
    mov pc, lr

.global sccf_lockCard
.type sccf_lockCard, %function
sccf_lockCard:
	movs r0, #1
	b sccf_changeMode

.global sccf_unlockCard
.type sccf_unlockCard, %function
sccf_unlockCard:
	movs r0, #3
	b sccf_changeMode

.pool
