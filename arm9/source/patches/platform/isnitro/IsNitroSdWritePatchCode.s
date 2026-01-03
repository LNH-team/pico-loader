.cpu arm7tdmi
.section "sdwrite", "ax"
.syntax unified
.thumb

// for armv4t, calling code must be thumb
// r0 = dst sector
// r1 = src
// r2 = sector count
.global sdwrite_asm
.type sdwrite_asm, %function
sdwrite_asm:
    push {r4-r7,lr}

    ldr r6,= 0x04000200
    // clear the bottom 8 bits of REG_EXMEMCNT to allow access to the GBA slot
    // and to set the right timings
    strb r6, [r6, #4]

    // load the GBA memory address as set by the debug monitor rom
    ldr r5, sdwrite_asm_agbRamPtr

    // setup write command
    lsrs r3, r6, #10 // 0x10000
    adds r5, r3 // GBA memory + 0x10000
    movs r3, #2
    strh r3, [r5, #2] // command = write
    str r0, [r5, #4] // sector address
    str r2, [r5, #8] // sector count

    mov lr, r5
    adds r5, #0x20 // data buffer in GBA slot area
    lsls r0, r2, #5 // sector count * 32; 512 bytes per sector = 32 iterations per sector with 16 bytes copied per iteration
4:
    ldmia r1!, {r2,r3,r4,r7}
    stmia r5!, {r2,r3,r4,r7}
    subs r0, #1
    bne 4b
    mov r5, lr

    // GBA slot irq bit: 1 << 13
    movs r4, #1
    lsls r3, r4, #13

    // disable GBA slot irq
    ldr r0, [r6, #0x10] // REG_IE
    bics r0, r3
    str r0, [r6, #0x10] // REG_IE

    // ack GBA slot irq
    str r3, [r6, #0x14] // REG_IF

    movs r4, #0x55
    strh r4, [r5] // status = cmd ready to be processed

    // poll for GBA slot irq in REG_IF
2:
    ldr r4, [r6, #0x14]  // REG_IF
    tst r4, r3
    beq 2b

    // ack GBA slot irq
    str r3, [r6, #0x14]

    // map back GBA slot to arm7
    movs r0, #0x80
    strb r0, [r6, #4]

    pop {r4-r7,pc} // popping pc is safe for armv4t when not switching mode

.balign 4

.global sdwrite_asm_agbRamPtr
sdwrite_asm_agbRamPtr:
    .word 0

.pool

.end