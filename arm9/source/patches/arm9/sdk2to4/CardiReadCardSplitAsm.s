.cpu arm946e-s
.section "patch_cardireadcard_split", "ax"
.syntax unified

.thumb
.global patch_cardireadcard_split_entry
.type patch_cardireadcard_split_entry, %function
patch_cardireadcard_split_entry:

.global patch_cardireadcard_split_return_offset
patch_cardireadcard_split_return_offset:
    movs r0, #0x38
    add lr, r0
    push {r1,r2,r3,r4,r6,lr}
    ldr r3, __patch_cardireadcard_split_fix_cp15_asm_address
    blx r3
.global patch_cardireadcard_split_mov_src_to_r0
patch_cardireadcard_split_mov_src_to_r0:
    nop
    ldr r3, __patch_cardireadcard_split_rom_offset_to_sd_sector_asm_address
    blx r3
    movs r2, #1 // r2 = sector count
.global patch_cardireadcard_split_mov_dst_to_r1
patch_cardireadcard_split_mov_dst_to_r1:
    mov r1, r8 // r1 = dst
.global patch_cardireadcard_split_mov_cardicommon_to_r6
patch_cardireadcard_split_mov_cardicommon_to_r6:
    movs r6, r4 // r6 = cardi_common
.global patch_cardireadcard_split_adjust_cardicommon_offset
patch_cardireadcard_split_adjust_cardicommon_offset:
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

.global patch_cardireadcard_split_mov_r3_to_dst
patch_cardireadcard_split_mov_r3_to_dst:
    mov r8, r3

    ldr r3, [r6, #0x24]
    subs r3, r3, r4
    str r3, [r6, #0x24]

do_read:
    lsrs r3, r1, #24 // if dst address is invalid (close to zero), ignore the read
    beq ignore_read  // this is intended to fix reads to null pointers that would be ignored if done with DMA

    ldr r3, __patch_cardireadcard_split_sdread_asm_address
    blx r3

ignore_read:
    ldr r3, splitReadNext
    bx r3
.balign 4
.global splitReadNext
splitReadNext: .word 0

.balign 4

.global __patch_cardireadcard_split_fix_cp15_asm_address
__patch_cardireadcard_split_fix_cp15_asm_address:
    .word 0

.global __patch_cardireadcard_split_rom_offset_to_sd_sector_asm_address
__patch_cardireadcard_split_rom_offset_to_sd_sector_asm_address:
    .word 0

.global __patch_cardireadcard_split_sdread_asm_address
__patch_cardireadcard_split_sdread_asm_address:
    .word 0

.pool


// Each independently relocated piece uses local literals and absolute links.
.section "patch_split_ack", "ax"
.thumb
.global splitAckEntry
.thumb_func
splitAckEntry:
    push {r0}
    ldr r0, ackIpc
    ldrh r3, [r0]
    movs r1, #15
    ands r1, r3
    cmp r1, #14
    bne ackNoRequest
    ldr r1, ackMask
    bics r3, r1
    ldr r1, ackValue
    orrs r3, r1
    push {r0,r3}
    // Stop game transfers before removing their VRAM destinations.
    // This runs only after a confirmed return request.
    ldr r0, ackDmaControl
    movs r1, #0
    str r1, [r0]
    str r1, [r0,#12]
    str r1, [r0,#24]
    str r1, [r0,#36]
    ldr r0, ackVram
    movs r1, #0x82
    strb r1, [r0]
    movs r1, #0x8A
    strb r1, [r0,#1]
    ldr r0, ackExmem
    ldrh r1, [r0]
    ldr r3, ackOwner
    orrs r1, r3
    strh r1, [r0]
    pop {r0,r3}
    strh r3, [r0]
    ldr r3, splitAckNext
    bx r3
ackNoRequest:
    ldr r1, ackMask
    bics r3, r1
    strh r3, [r0]
    pop {r0}
    pop {r1,r2,r3,r4,r6,pc}
.balign 4
ackIpc: .word 0x04000180
ackMask: .word 0xF00
ackValue: .word 0xD00
ackDmaControl: .word 0x040000B8
ackVram: .word 0x04000242
ackExmem: .word 0x04000204
ackOwner: .word 0x800
.global splitAckNext
splitAckNext: .word 0

.section "patch_split_park", "ax"
.thumb
.global splitParkEntry
.thumb_func
splitParkEntry:
    ldr r0, parkIme
    movs r1, #0
    str r1, [r0]
    ldr r0, parkMain
    ldr r1, parkBlack
    strh r1, [r0]
    ldr r0, parkSub
    strh r1, [r0]
parkWait:
    ldr r0, parkIpc
    ldrh r3, [r0]
    movs r1, #15
    ands r3, r1
    cmp r3, #10
    bne parkWait
    ldr r3, splitParkLoad
    blx r3
    cmp r0, #1
    beq parkStart
parkFailed:
    b parkFailed
parkStart:
    ldr r0, parkMain
    movs r1, #0
    strh r1, [r0]
    ldr r0, parkSub
    strh r1, [r0]
    ldr r0, parkBase
    bx r0
.balign 4
parkIme: .word 0x04000208
parkMain: .word 0x0400006C
parkSub: .word 0x0400106C
parkBlack: .word 0x8010
parkIpc: .word 0x04000180
parkBase: .word 0x06800000
.global splitParkLoad
splitParkLoad: .word 0

.section "patch_split_load", "ax"
.thumb
.global splitLoadEntry
.thumb_func
splitLoadEntry:
    push {r4-r7,lr}
    ldr r0, loadExmem
    ldrh r1, [r0]
    ldr r2, loadOwner
    bics r1, r2
    strh r1, [r0]
    ldr r0, loadVram
    movs r1, #0x80
    strb r1, [r0]
    ldr r4, splitLoadExtents
    ldr r5, loadBase
    movs r6, #0
loadExtent:
    ldmia r4!, {r0,r2}
    cmp r0, #0
    beq loadCheck
    movs r7, r0
    adds r6, r6, r7
    movs r3, #128
    lsls r3, #1
    cmp r6, r3
    bhi loadBad
    movs r0, r2
    movs r1, r5
    movs r2, r7
    ldr r3, splitLoadRead
    blx r3
    lsls r7, #9
    adds r5, r5, r7
    b loadExtent
loadCheck:
    ldr r3, splitLoadCheck
    bx r3
loadBad:
    movs r0, #0
    pop {r4-r7,pc}
.balign 4
loadExmem: .word 0x04000204
loadOwner: .word 0x800
loadVram: .word 0x04000240
loadBase: .word 0x06800000
.global splitLoadExtents
splitLoadExtents: .word 0
.global splitLoadRead
splitLoadRead: .word 0
.global splitLoadCheck
splitLoadCheck: .word 0

.section "patch_split_check", "ax"
.thumb
.global splitCheckEntry
.thumb_func
splitCheckEntry:
    cmp r6, #0
    beq checkBad
    ldr r0, checkBase
    adr r1, checkWords
    movs r2, #4
checkLoop:
    ldr r3, [r0]
    ldr r7, [r1]
    cmp r3, r7
    bne checkBad
    adds r0, #4
    adds r1, #4
    subs r2, #1
    bne checkLoop
    movs r0, #1
    pop {r4-r7,pc}
checkBad:
    movs r0, #0
    pop {r4-r7,pc}
.balign 4
checkBase: .word 0x06800000
checkWords: .word 0xE59F011C,0xE5C00000,0xE59F0118,0xEE010F10
.end
