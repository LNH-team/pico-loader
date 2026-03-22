.cpu arm7tdmi
.section "patch_cheatengine", "ax"
.syntax unified

// For reference on how Action Replay codes work for the DS
// - https://problemkaputt.de/gbatek-ds-cart-cheat-action-replay-ds.htm
// - https://github.com/melonDS-emu/melonDS/blob/master/src/AREngine.cpp

.arm
.global cheatengine_entry_arm
.type cheatengine_entry_arm, %function
cheatengine_entry_arm:
    adr r12, cheatengine_entry + 1
    bx r12

.thumb
.global cheatengine_entry
.type cheatengine_entry, %function
cheatengine_entry:
    pop {r0, r1}
    mov lr, r1
    push {r4, r5, lr}

    // increment 16-bit counter for C5
    adr r1, c5counter
    ldrh r0, [r1]
    adds r0, #1
    strh r0, [r1]

    ldr r4, cheatengine_cheatsPtr
    ldr r5, [r4, #4] // pload_cheats_t::numberOfCheats
    adds r4, #8
entry_cheats_loop:
    subs r5, #1
    bmi entry_end
    movs r0, r4
    bl cheatengine_runCheat
    ldmia r4!, {r0} // r0 = length of cheat in bytes (pload_cheat_t::length)
    adds r4, r0
    b entry_cheats_loop
entry_end:
    pop {r4, r5}
    pop {r3}
    bx r3

runCheat_end:
    pop {r4, r5, r6} // r8, r9, r10
    mov r8, r4
    mov r9, r5
    mov r10, r6
    pop {r4, r5, r6, r7, pc}

cheatengine_runCheat:
    push {r4, r5, r6, r7, lr}
    mov r4, r8
    mov r5, r9
    mov r6, r10
    push {r4, r5, r6} // r8, r9, r10

    ldmia r0!, {r1} // r1 = length of cheat code
    adds r1, r0
    mov r8, r1 // r8 = end of cheat code
    mov r9, r0 // r9 = loop start
    movs r4, #0 // r4 = loop counter
    movs r5, #0 // r5 = offset register
    movs r6, #0 // r6 = data register
    movs r7, #1 // r7 = condition stack
    mov r10, r7 // r10 = loop condition stack backup
runCheat_opcode_loop:
    cmp r0, r8
    bhs runCheat_end

    ldmia r0!, {r1, r2} // r1, r2 = cheat code a, b

    // == condition check ==

    lsrs r3, r1, #24 // r3 = op
    subs r3, #0xD0
    cmp r3, #2
    bls 1f // D0 - D2 are not condition checked

    lsrs r3, r7, #1 // check bottom bit of condition stack
    bcs 1f // if set, the opcode is executed

    lsrs r3, r1, #24 // r3 = op
    cmp r3, #0xC2
    beq 2f
    cmp r3, #0xE0
    bne runCheat_opcode_loop

2:
    // opcode C2 and EX have a dynamic length
    adds r2, #7
    movs r3, #7
    bics r2, r3 // pad length to multiple of 8
    adds r0, r2

    b runCheat_opcode_loop

1:
    // == execute the opcode ==
    lsrs r3, r1, #28 // r3 = top 4 bits of op
    lsls r1, r1, #4
    lsrs r1, r1, #4
    lsls r3, r3, #1
    add r3, pc
    ldrh r3, [r3, #2]
    add pc, r3

top_4_bits_table:
    .short (opcode_0X - top_4_bits_table - 2)
    .short (opcode_1X - top_4_bits_table - 2)
    .short (opcode_2X - top_4_bits_table - 2)
    .short (opcode_3X - top_4_bits_table - 2)
    .short (opcode_4X - top_4_bits_table - 2)
    .short (opcode_5X - top_4_bits_table - 2)
    .short (opcode_6X - top_4_bits_table - 2)
    .short (opcode_7X - top_4_bits_table - 2)
    .short (opcode_8X - top_4_bits_table - 2)
    .short (opcode_9X - top_4_bits_table - 2)
    .short (opcode_AX - top_4_bits_table - 2)
    .short (opcode_BX - top_4_bits_table - 2)
    .short (opcode_CX - top_4_bits_table - 2)
    .short (opcode_DX - top_4_bits_table - 2)
    .short (opcode_EX - top_4_bits_table - 2)
    .short (opcode_FX - top_4_bits_table - 2)

opcode_0X: // 32-bit write
    str r2, [r1, r5]
    b runCheat_opcode_loop

opcode_1X: // 16-bit write
    strh r2, [r1, r5]
    b runCheat_opcode_loop

opcode_2X: // 8-bit write
    strb r2, [r1, r5]
    b runCheat_opcode_loop

opcode_3X: // IF b > u32[a]
    bl prepare_opcode_3X_to_6X
    bls runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_4X: // IF b < u32[a]
    bl prepare_opcode_3X_to_6X
    bhs runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_5X: // IF b == u32[a]
    bl prepare_opcode_3X_to_6X
    bne runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_6X: // IF b != u32[a]
    bl prepare_opcode_3X_to_6X
    beq runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

prepare_opcode_3X_to_6X:
    cmp r1, #0
    bne 1f
    movs r1, r5
1:
    ldr r1, [r1]
    lsls r7, r7, #1
    cmp r2, r1
    bx lr

opcode_7X: // IF b.l > ((~b.h) & u16[a])
    bl prepare_opcode_7X_to_AX
    bls runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_8X: // IF b.l < ((~b.h) & u16[a])
    bl prepare_opcode_7X_to_AX
    bhs runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_9X: // IF b.l == ((~b.h) & u16[a])
    bl prepare_opcode_7X_to_AX
    bne runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

opcode_AX: // IF b.l != ((~b.h) & u16[a])
    bl prepare_opcode_7X_to_AX
    beq runCheat_opcode_loop
    adds r7, #1
    b runCheat_opcode_loop

prepare_opcode_7X_to_AX:
    cmp r1, #0
    bne 1f
    movs r1, r5
1:
    ldrh r1, [r1]
    lsls r7, r7, #1
    lsrs r3, r2, #16
    bics r1, r3
    lsls r2, r2, #16
    lsrs r2, r2, #16
    cmp r2, r1
    bx lr

opcode_BX: // offset = u32[a + offset]
    ldr r5, [r1, r5]
    b runCheat_opcode_loop

opcode_CX:
    lsrs r3, r1, #24 // r3 = op
    cmp r3, #0
    beq opcode_C0
    cmp r3, #2
    beq opcode_C2
    cmp r3, #4
    beq opcode_C4
    cmp r3, #5
    beq opcode_C5
    cmp r3, #6
    beq opcode_C6
    b runCheat_opcode_loop

opcode_C0: // FOR 0..b
    movs r4, r2 // loop count
    mov r9, r0 // loop start
    mov r10, r7 // condition stack backup
    b runCheat_opcode_loop

opcode_C2: // execute code from cheat list
    movs r3, r0

    // pad length to multiple of 8
    adds r2, r2, #7
    lsrs r2, r2, #3
    lsls r2, r2, #3
    adds r0, r2

    lsrs r1, r1, #1
    bcc 1f
    adds r3, #1 // set thumb bit
1:
    push {r0}
    bl 2f
    pop {r0}
    b runCheat_opcode_loop
2:
    bx r3

opcode_C4: // offset = pointer to C4000000 opcode
    movs r5, r0
    subs r5, #8
    b runCheat_opcode_loop

opcode_C5: // IF (count & b.l) == b.h
    ldr r3, c5counter
    lsls r7, r7, #1
    lsrs r1, r2, #16 // chk
    lsls r2, r2, #16
    lsrs r2, r2, #16 // mask
    ands r3, r2
    cmp r3, r1
    bne 1f
    adds r7, #1
1:
    b runCheat_opcode_loop

opcode_C6: // u32[b] = offset
    str r5, [r2]
    b runCheat_opcode_loop

opcode_DX:
    lsrs r3, r1, #24
    cmp r3, #0xD
        bhs opcode_DX_invalid
    lsls r3, r3, #1
    add r3, pc
    ldrh r3, [r3, #2]
    add pc, r3

DX_table:
    .short (opcode_D0 - DX_table - 2)
    .short (opcode_D1 - DX_table - 2)
    .short (opcode_D2 - DX_table - 2)
    .short (opcode_D3 - DX_table - 2)
    .short (opcode_D4 - DX_table - 2)
    .short (opcode_D5 - DX_table - 2)
    .short (opcode_D6 - DX_table - 2)
    .short (opcode_D7 - DX_table - 2)
    .short (opcode_D8 - DX_table - 2)
    .short (opcode_D9 - DX_table - 2)
    .short (opcode_DA - DX_table - 2)
    .short (opcode_DB - DX_table - 2)
    .short (opcode_DC - DX_table - 2)

opcode_D0: // ENDIF
    lsrs r7, r7, #1
opcode_DX_invalid:
    b runCheat_opcode_loop

opcode_D1: // NEXT
    cmp r4, #0
    ble 1f

    subs r4, #1
    mov r0, r9 // jump to loop start
    b runCheat_opcode_loop

1:
    mov r7, r10 // restore condition stack backup
    b runCheat_opcode_loop

opcode_D2: // NEXT+FLUSH
    cmp r4, #0
    ble 1f

    subs r4, #1
    mov r0, r9 // jump to loop start
    b runCheat_opcode_loop

1:
    movs r5, #0
    movs r6, #0
    movs r7, #1
    b runCheat_opcode_loop

opcode_D3: // offset = b
    movs r5, r2
    b runCheat_opcode_loop

opcode_D4: // data op
    lsls r1, r1, #24
    lsrs r1, r1, #22
    cmp r1, #(9 << 2) // if data op >= 9, ignore
        bhs opcode_D4_invalid
    add pc, r1
    nop

opcode_D4_0: // datareg += b
    adds r6, r2
opcode_D4_invalid:
    b runCheat_opcode_loop

opcode_D4_1: // datareg |= b
    orrs r6, r2
    b runCheat_opcode_loop

opcode_D4_2: // datareg &= b
    ands r6, r2
    b runCheat_opcode_loop

opcode_D4_3: // datareg ^= b
    eors r6, r2
    b runCheat_opcode_loop

opcode_D4_4: // datareg >>= b
    lsls r6, r6, r2
    b runCheat_opcode_loop

opcode_D4_5: // datareg <<= b
    lsrs r6, r6, r2
    b runCheat_opcode_loop

opcode_D4_6: // datareg = ROR(datareg, b)
    rors r6, r2
    b runCheat_opcode_loop

opcode_D4_7: // (s32)datareg >>= b
    asrs r6, r6, r2
    b runCheat_opcode_loop

opcode_D4_8: // datareg *= b
    muls r6, r2
    b runCheat_opcode_loop

opcode_D5: // datareg = b
    movs r6, r2
    b runCheat_opcode_loop

opcode_D6: // u32[b+offset] = datareg / offset += 4
    str r6, [r2, r5]
    adds r5, #4
    b runCheat_opcode_loop

opcode_D7: // u16[b+offset] = datareg / offset += 2
    strh r6, [r2, r5]
    adds r5, #2
    b runCheat_opcode_loop

opcode_D8: // u8[b+offset] = datareg / offset += 1
    strb r6, [r2, r5]
    adds r5, #1
    b runCheat_opcode_loop

opcode_D9: // datareg = u32[b+offset]
    ldr r6, [r2, r5]
    b runCheat_opcode_loop

opcode_DA: // datareg = u16[b+offset]
    ldrh r6, [r2, r5]
    b runCheat_opcode_loop

opcode_DB: // datareg = u8[b+offset]
    ldrb r6, [r2, r5]
    b runCheat_opcode_loop

opcode_DC: // offset += b
    adds r5, r2
    b runCheat_opcode_loop

opcode_EX: // copy b param bytes to address a+offset
    push {r0}

    // pad length to multiple of 8
    adds r3, r2, #7
    lsrs r3, r3, #3
    lsls r3, r3, #3
    adds r0, r3

    pop {r3}
    push {r0}
    adds r1, r5
    b FX_word_loop

opcode_FX: // copy b bytes from address offset to address a
    push {r0}
    movs r3, r5
FX_word_loop:
    cmp r2, #4
    blo FX_byte_loop
    ldmia r3!, {r0}
    stmia r1!, {r0}
    subs r2, #4
    b FX_word_loop

FX_byte_loop:
    cmp r2, #1
    blo FX_end
    ldrb r0, [r3]
    adds r3, #1
    strb r0, [r1]
    adds r1, #1
    subs r2, #1
    b FX_byte_loop

FX_end:
    pop {r0}
    b runCheat_opcode_loop

.balign 4

c5counter:
    .word 0

.global cheatengine_cheatsPtr
cheatengine_cheatsPtr:
    .word 0

.pool
.end