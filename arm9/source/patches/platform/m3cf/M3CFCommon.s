.cpu arm7tdmi
.syntax unified
.thumb
.section "m3cf_change_mode", "ax"

.macro READ_REG regAddr
	ldr r1, =\regAddr
	ldrh r2, [r1]
.endm
.equ M3_MODE_ROM, 8
.equ M3_MODE_MEDIA, 6

.global m3cf_lockUnlockCard
.type m3cf_lockUnlockCard, %function
m3cf_lockUnlockCard:
    cmp r0, #1
    bne unlock
	movs r0, #8
M3_changeMode:
    push {r1-r3, lr}
	READ_REG #0x08e00002
	READ_REG #0x0800000e
	READ_REG #0x08801ffc
	READ_REG #0x0800104a
	READ_REG #0x08800612
	READ_REG #0x08000000
	READ_REG #0x08801b66
	ldr r1, =#0x08800000
	adds r1, r0
	ldrh r2, [r1]
	READ_REG #0x0800080e
	READ_REG #0x08000000
	cmp r0, M3_MODE_ROM
	bne lastRomRead
	READ_REG #0x09000000
    pop {r1-r3, pc}
lastRomRead:
	READ_REG #0x080001e4
	READ_REG #0x080001e4
	READ_REG #0x08000188
	READ_REG #0x08000188
unlock:
	movs r0, #6
    b M3_changeMode
.pool
