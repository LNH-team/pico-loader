.cpu arm7tdmi
.syntax unified
.thumb

.section "m3cf_lock_unlock", "ax"

.equ M3_MODE_ROM, 8
.equ M3_MODE_MEDIA, 6

.global m3cf_lockUnlockCard
.type m3cf_lockUnlockCard, %function
m3cf_lockUnlockCard:
    cmp r0, #1
    bne unlock
    movs r0, #0x08
M3_changeMode:
    push {r1-r7, lr}
    @ READ_REG #0x08e00002
    ldr r1, =#0x08e00002
    ldrh r1, [r1]
    
    adr r1, M3_regs
    @ r2 has #0x08801ffc
    @ r3 has #0x0800104a
    @ r4 has #0x08800612
    @ r5 has #0x08801b66
    @ r6 has #0x0800080e
    @ r7 has #0x09000000
    ldm r1!, {r2-r7}

    @ load 0x08000000
    movs r1, #1
    lsls r1, #27

    @ READ_REG #0x0800000e
    ldrh r1, [r1, #0x0E]

    @ READ_REG #0x08801ffc
    ldrh r1, [r2]
    @ READ_REG #0x0800104a
    ldrh r1, [r3]
    @ READ_REG #0x08800612
    ldrh r1, [r4]

    @ READ_REG #0x08000000
    @ load 0x08000000
    movs r2, #1
    lsls r2, #27
    ldrh r1, [r2]

    @ READ_REG #0x08801b66
    ldrh r1, [r5]

    @ READ_REG #0x08800000 + r0
    @ ldr r1, =#0x08800000
	movs r1, 0x88
	lsls r1, #20
    ldrh r1, [r1, r0]

    @ READ_REG #0x0800080e
    ldrh r1, [r6]

    @ READ_REG #0x08000000
    ldrh r1, [r2]

    cmp r0, M3_MODE_ROM
    beq lastRomRead
    @ READ_REG #0x09000000
    ldrh r1, [r7]
    pop {r1-r7, pc}

lastRomRead:
    @ READ_REG #0x080001e4
    ldr r1, =#0x080001e4
    ldrh r3, [r1]
    ldrh r3, [r1]
    @ READ_REG #0x08000188
    subs r1, #0x5C
    ldrh r3, [r1]
    ldrh r3, [r1]
    pop {r1-r7, pc}
unlock:
    movs r0, #0x06
    b M3_changeMode
.pool
M3_regs:
    .word 0x08801ffc
    .word 0x0800104a
    .word 0x08800612
    .word 0x08801b66
    .word 0x0800080e
    .word 0x09000000
