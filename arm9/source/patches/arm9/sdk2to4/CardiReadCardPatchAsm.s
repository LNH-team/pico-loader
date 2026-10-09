.cpu arm946e-s
.section "patch_cardireadcard", "ax"
.syntax unified

.thumb
.global patch_cardireadcard_entry
.type patch_cardireadcard_entry, %function
patch_cardireadcard_entry:

.global patch_cardireadcard_return_offset
patch_cardireadcard_return_offset:
    movs r0, #0x38
    add lr, r0
    push {r1,r2,r3,r4,r6,lr}
    ldr r3, __patch_cardireadcard_fix_cp15_asm_address
    blx r3
.global patch_cardireadcard_mov_src_to_r0
patch_cardireadcard_mov_src_to_r0:
    nop
    ldr r3, __patch_cardireadcard_rom_offset_to_sd_sector_asm_address
    blx r3
    movs r2, #1 // r2 = sector count
.global patch_cardireadcard_mov_dst_to_r1
patch_cardireadcard_mov_dst_to_r1:
    mov r1, r8 // r1 = dst
.global patch_cardireadcard_mov_cardicommon_to_r6
patch_cardireadcard_mov_cardicommon_to_r6:
    movs r6, r4 // r6 = cardi_common
.global patch_cardireadcard_adjust_cardicommon_offset
patch_cardireadcard_adjust_cardicommon_offset:
    subs r6, #0
    ldr r4, [r6, #0x20] // r4 = actual dst
    cmp r4, r1
    bne do_read // if dst != actual dst the cache is in use

    ldr r2, [r6, #0x24] // remaining length
    lsrs r2, r2, #9 // sectors left to read
    cmp r2, lr
    ble 1f
    mov r2, lr // clip to cluster boundary
1:
    subs r4, r2, #1
    lsls r4, r4, #9
    ldr r3, [r6, #0x1C]
    adds r3, r3, r4
    str r3, [r6, #0x1C]
    ldr r3, [r6, #0x20]
    adds r3, r3, r4
    str r3, [r6, #0x20]

.global patch_cardireadcard_mov_r3_to_dst
patch_cardireadcard_mov_r3_to_dst:
    mov r8, r3

    ldr r3, [r6, #0x24]
    subs r3, r3, r4
    str r3, [r6, #0x24]

do_read:
    lsrs r3, r1, #24 // if dst address is invalid (close to zero), ignore the read
    beq ignore_read  // this is intended to fix reads to null pointers that would be ignored if done with DMA

    ldr r3, __patch_cardireadcard_sdread_asm_address
    blx r3

.global patch_cardireadcard_retail_entry
.thumb_func
patch_cardireadcard_retail_entry:
ignore_read:
    // Test 100: acknowledge ARM7, then deliberately park ARM9.
    // Preserve r0 result; r1/r3 restored by the original epilogue.
    push {r0}
    ldr r0, retailIpcSync
    ldrh r3, [r0]
    movs r1, #15
    ands r1, r3
    cmp r1, #14
    bne retail_clear_ack
    ldr r1, retailOutputMask
    bics r3, r1
    ldr r1, retailAckValue
    orrs r3, r1
    b retail_write_ack
retail_clear_ack:
    ldr r1, retailOutputMask
    bics r3, r1
retail_write_ack:
    // Test 101: give VRAM C to ARM7 before publishing acknowledgement.
    movs r1, #15
    ands r1, r3
    cmp r1, #14
    bne retail_publish_ack
    push {r0,r3}
    ldr r0, retailVramC
    movs r1, #0x82
    strb r1, [r0]
    movs r1, #0x8A // map VRAM D at ARM7 +128 KiB for loader heap
    strb r1, [r0, #1]
    // Test 103: transfer Slot-1 ownership to ARM7 before acknowledgement.
    ldr r0, retailExmemCnt
    ldrh r1, [r0]
    ldr r3, retailArm7CardOwner
    orrs r1, r3
    strh r1, [r0]
    pop {r0,r3}
retail_publish_ack:
    strh r3, [r0]
    movs r1, #15
    ands r1, r3
    cmp r1, #14
    bne retail_return
    // Deliberate takeover marker: stop ARM9 IRQs and game execution.
    // ARM7 continues independently and can observe the D acknowledgement.
    ldr r0, retailIme
    movs r1, #0
    str r1, [r0]
    ldr r0, retailBrightMain
    ldr r1, retailBlackValue
    strh r1, [r0]
    ldr r0, retailBrightSub
    strh r1, [r0]
retail_park:
    // Test 105: observe the unmodified real ARM7 loader's startup signal.
    ldr r0, retailIpcSync
    ldrh r3, [r0]
    movs r1, #15
    ands r3, r1
    cmp r3, #10
    bne retail_park
    // Test 106: stage real ARM9 loader before marking success.
    bl retail_load_arm9
    cmp r0, #1
    beq retail_arm9_staged
retail_load_failed:
    b retail_load_failed
retail_arm9_staged:
    // Test 107: restore visible output and enter matching ARM9 loader.
    ldr r0, retailBrightMain
    movs r1, #0
    strh r1, [r0]
    ldr r0, retailBrightSub
    strh r1, [r0]
    ldr r0, retailArm9Base
    bx r0
retail_return:
    pop {r0}
    pop {r1,r2,r3,r4,r6,pc}

.balign 4
retailIpcSync:
    .word 0x04000180
retailOutputMask:
    .word 0x00000F00
retailAckValue:
    .word 0x00000D00
retailIme:
    .word 0x04000208
retailBrightMain:
    .word 0x0400006C
retailBrightSub:
    .word 0x0400106C
retailBlackValue:
    .word 0x00008010
retailWhiteValue:
    .word 0x00004010
retailVramC:
    .word 0x04000242
retailExmemCnt:
    .word 0x04000204
retailArm7CardOwner:
    .word 0x00000800

.thumb
.type retail_load_arm9, %function
retail_load_arm9:
    push {r4-r7,lr}
    // ARM7 is waiting in initIpc and no longer reading the card.
    ldr r0, retailLoadExmemCnt
    ldrh r1, [r0]
    ldr r2, retailLoadCardOwner
    bics r1, r2
    strh r1, [r0]
    ldr r0, retailVramA
    movs r1, #0x80
    strb r1, [r0]
    adr r4, patch_cardireadcard_loader9Extents
    ldr r5, retailArm9Base
    movs r6, #0
retail_load_extent:
    ldmia r4!, {r0,r2} // sector count, physical start
    cmp r0, #0
    beq retail_check_arm9
    movs r7, r0
    adds r6, r6, r7
    movs r3, #128
    lsls r3, #1
    cmp r6, r3
    bhi retail_load_bad
    movs r0, r2
    movs r1, r5
    movs r2, r7
    ldr r3, __patch_cardireadcard_sdread_asm_address
    blx r3
    lsls r7, #9
    adds r5, r5, r7
    b retail_load_extent
retail_check_arm9:
    cmp r6, #0
    beq retail_load_bad
    ldr r0, retailArm9Base
    adr r1, retailArm9EntryWords
    movs r2, #4
retail_entry_check:
    ldr r3, [r0]
    ldr r7, [r1]
    cmp r3, r7
    bne retail_load_bad
    adds r0, #4
    adds r1, #4
    subs r2, #1
    bne retail_entry_check
    movs r0, #1
    pop {r4-r7,pc}
retail_load_bad:
    movs r0, #0
    pop {r4-r7,pc}

.balign 4
retailLoadExmemCnt:
    .word 0x04000204
retailLoadCardOwner:
    .word 0x00000800
retailVramA:
    .word 0x04000240
retailArm9Base:
    .word 0x06800000
retailArm9EntryWords:
    .word 0xE59F011C, 0xE5C00000, 0xE59F0118, 0xEE010F10
.global patch_cardireadcard_loaderParams
patch_cardireadcard_loaderParams:
    .space 132 // Test 116: u32 boot drive, bounded 128-byte launcher path

.global patch_cardireadcard_loader9Extents
patch_cardireadcard_loader9Extents:
    .space 40


.global __patch_cardireadcard_fix_cp15_asm_address
__patch_cardireadcard_fix_cp15_asm_address:
    .word 0

.global __patch_cardireadcard_rom_offset_to_sd_sector_asm_address
__patch_cardireadcard_rom_offset_to_sd_sector_asm_address:
    .word 0

.global __patch_cardireadcard_sdread_asm_address
__patch_cardireadcard_sdread_asm_address:
    .word 0

.pool

.end