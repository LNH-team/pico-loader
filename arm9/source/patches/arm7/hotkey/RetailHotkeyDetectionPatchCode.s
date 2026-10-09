.cpu arm7tdmi
.section "patch_retailhotkeydetect", "ax"
.syntax unified

.arm
.global patch_retailhotkeydetect_entry_arm
.type patch_retailhotkeydetect_entry_arm, %function
patch_retailhotkeydetect_entry_arm:
    adr r12, patch_retailhotkeydetect_entry + 1
    bx r12

.thumb
.global patch_retailhotkeydetect_entry
.type patch_retailhotkeydetect_entry, %function
patch_retailhotkeydetect_entry:
    pop {r0, r1}
    mov lr, r1
    push {r2-r5, lr}

    // Test 99: nonblocking acknowledgement check on each invocation.
    // A pending successful read is required before accepting ARM9's reply.
    adr r2, ackPending
    ldr r3, [r2]
    cmp r3, #0
    beq check_hotkey
    ldr r2, regIpcSync
    ldrh r3, [r2]
    movs r4, #15
    ands r3, r4
    cmp r3, #13
    bne check_hotkey
    adr r2, ackPending
    movs r3, #0
    str r3, [r2]
    // Test 101: acknowledged ARM9 has mapped VRAM C and parked.
    adr r2, stagingCursor
    ldr r3, stagingBase
    str r3, [r2]
    bl patch_retailhotkeydetect_dspico_loader_probe
    adr r2, stagingCursor
    movs r3, #0
    str r3, [r2]
    cmp r0, #0
    beq detection_done

    // Verify the complete retained payload after all sector writes.
    ldr r2, stagingBase
    ldr r3, loaderWordCount
    movs r4, #0
staging_verify:
    ldr r5, [r2]
    adds r4, r4, r5
    adds r2, #4
    subs r3, #1
    bne staging_verify
    ldr r2, expectedLoaderSum
    cmp r4, r2
    bne detection_done
    // Test 104: execute ARM instructions in unused VRAM C and return.
    bl execute_vram_token
    cmp r0, #0x5B
    bne detection_done
    // Test 105: enter actual ARM7 loader, never return to game callback.
    // Loader crt0 disables IRQs, clears BSS and establishes its own stack.
    // Real loaderMain clears sound and emits startup IPCSYNC A.
    bl setup_launcher_header
    cmp r0, #0
    beq detection_done
    ldr r2, stagingBase
    ldr r0, [r2] // pload_header7_t.entryPoint (ARM address)
    adr r2, arm7_loader_takeover
    bx r2

.balign 4
.arm
arm7_loader_takeover:
    // Test 108: abandon the game's IRQ context before real loader startup.
    // r0 remains the actual loader entry; IME off before CPU IRQ enable.
    ldr r1, takeoverIme
    mov r2, #0
    str r2, [r1]
    msr cpsr_c, #0xD2 // ARM IRQ mode, IRQ/FIQ masked
    ldr sp, takeoverIrqStack
    msr cpsr_c, #0xD3 // ARM supervisor mode, IRQ/FIQ masked
    ldr sp, takeoverSvcStack
    msr cpsr_c, #0x1F // ARM system mode; crt0 establishes system stack
    bx r0
.balign 4
takeoverIme:
    .word 0x04000208
takeoverIrqStack:
    .word 0x0380FF80
takeoverSvcStack:
    .word 0x0380FFC0

.thumb

check_hotkey:
    ldr r2, regKeyInput
    ldrh r3, [r2]
    ldr r4, hotkeyMask
    ands r3, r4
    bne clear_state

    adr r2, firedFlag
    ldr r3, [r2]
    cmp r3, #0
    bne detection_done

    adr r2, holdCounter
    ldr r3, [r2]
    adds r3, #1
    str r3, [r2]
    cmp r3, #30
    blo detection_done

    adr r2, firedFlag
    movs r3, #1
    str r3, [r2]

    // Test 136: request ownership before any loader SD reads. The game
    // may still be using the card here. Full read/checksum verification
    // remains in the post-ack path after ARM9 grants ownership.

    // Preserve IPC control/IRQ bits; change only ARM7's outgoing nibble.
    ldr r2, regIpcSync
    ldrh r3, [r2]
    ldr r4, ipcOutputMask
    bics r3, r4
    ldr r4, ipcReturnRequest
    orrs r3, r4
    strh r3, [r2]

    adr r2, ackPending
    movs r3, #1
    str r3, [r2]
    b detection_done

clear_state:
    adr r2, holdCounter
    movs r3, #0
    str r3, [r2]
    adr r2, firedFlag
    str r3, [r2]

detection_done:
    pop {r2-r5}
    pop {r3}
    bx r3

.balign 4
regKeyInput:
    .word 0x04000130
regSoundCnt:
    .word 0x04000500
regIpcSync:
    .word 0x04000180
ipcOutputMask:
    .word 0x00000F00
ipcReturnRequest:
    .word 0x00000E00
hotkeyMask:
    .word 0x00000384

holdCounter:
    .word 0
firedFlag:
    .word 0
ackPending:
    .word 0


.thumb
.type patch_retailhotkeydetect_dspico_loader_probe, %function
patch_retailhotkeydetect_dspico_loader_probe:
    push {r6, r7, lr}
    movs r6, #0              // rolling 32-bit word sum
    ldr r7, loaderWordCount
    adr r5, patch_retailhotkeydetect_loaderExtents

full_next_extent:
    ldmia r5!, {r3, r4}      // sector count, first physical sector
    cmp r3, #0
    beq full_failed

full_next_sector:
    push {r3, r5}            // preserve extent state across the SD helper
    bl read_one_sector
    cmp r0, #0
    beq full_read_failed

    adr r1, loaderSectorBuffer
    movs r2, #128
    cmp r7, r2
    bhs full_sum_words
    movs r2, r7              // final sector: ignore bytes beyond file EOF
full_sum_words:
    ldr r0, [r1]
    // Only the post-ack pass retains words in VRAM C.
    push {r4}
    adr r4, stagingCursor
    ldr r3, [r4]
    cmp r3, #0
    beq full_no_stage
    str r0, [r3]
    adds r3, #4
    str r3, [r4]
full_no_stage:
    pop {r4}
    adds r6, r6, r0
    adds r1, #4
    subs r7, #1
    subs r2, #1
    bne full_sum_words

    pop {r3, r5}
    cmp r7, #0
    beq full_check_sum
    adds r4, #1
    subs r3, #1
    bne full_next_sector
    b full_next_extent

full_check_sum:
    ldr r0, expectedLoaderSum
    cmp r6, r0
    bne full_failed         // Test 96: require full-file sum match again
    movs r0, #1
    pop {r6, r7, pc}

full_read_failed:
    pop {r3, r5}
full_failed:
    movs r0, #0
    pop {r6, r7, pc}

.balign 4
stagingCursor:
    .word 0
stagingBase:
    .word 0x06000000


.thumb
.type setup_launcher_header, %function
setup_launcher_header:
    ldr r2, patch_retailhotkeydetect_loaderParamsAddress
    cmp r2, #0
    beq launcher_params_bad
    ldr r0, launcherHeaderBase
    ldr r3, [r2]
    strh r3, [r0, #8] // boot drive; apiVersion at +10 stays intact
    ldr r3, patch_retailhotkeydetect_dldiAddress
    str r3, [r0, #4] // persisted original storage driver
    adds r2, #4
    ldrb r3, [r2]
    cmp r3, #0
    beq launcher_params_bad
    ldr r4, launcherRomPath
    ldr r5, launcherReturnPath
    ldr r1, patch_retailhotkeydetect_pathWords // full=32, split=16; header tail remains zero
launcher_copy_path:
    ldr r3, [r2]
    str r3, [r4]
    str r3, [r5]
    adds r2, #4
    adds r4, #4
    adds r5, #4
    subs r1, #1
    bne launcher_copy_path
    // Fresh disk header has empty save/argv/cheats. The persisted real
    // DLDI driver can now be relocated and patched into the launcher.
    movs r0, #1
    bx lr
launcher_params_bad:
    movs r0, #0
    bx lr
.balign 4
.global patch_retailhotkeydetect_pathWords
patch_retailhotkeydetect_pathWords:
    .word 32
.global patch_retailhotkeydetect_loaderParamsAddress
patch_retailhotkeydetect_loaderParamsAddress:
    .word 0
.global patch_retailhotkeydetect_dldiAddress
patch_retailhotkeydetect_dldiAddress:
    .word 0
launcherHeaderBase:
    .word 0x06000000
launcherRomPath:
    .word 0x0600000C
launcherReturnPath:
    .word 0x06000310

.thumb
.type execute_vram_token, %function
execute_vram_token:
    push {lr}
    ldr r2, vramTokenAddress
    ldr r3, vramTokenMov
    str r3, [r2]
    ldr r3, vramTokenBx
    str r3, [r2, #4]
    adr r3, vram_token_return
    adds r3, #1
    mov lr, r3
    bx r2
.balign 4
vram_token_return:
    pop {pc}
.balign 4
vramTokenAddress:
    .word 0x06010000
vramTokenMov:
    .word 0xE3A0005B // ARM mov r0, #0x5B
vramTokenBx:
    .word 0xE12FFF1E // ARM bx lr; return to Thumb

// Read physical SD sector r4 through DSpico E3/E4/E5 into loaderSectorBuffer.
// Returns r0=1 on success, r0=0 on timeout/failure. r4 is preserved.
read_one_sector:
    push {r4, lr}
    ldr r2, cardRegBase

    ldr r5, probeTimeout
read_wait_idle:
    ldrb r3, [r2, #0x0F]
    lsrs r3, r3, #8
    bcc read_idle
    subs r5, #1
    bne read_wait_idle
    b read_one_failed

read_idle:
    movs r3, #0x80
    strb r3, [r2, #0x09]

    movs r3, #0xE3
    str r3, [r2, #0x10]
    movs r3, r4
    rors r3, r2
    str r3, [r2, #0x14]
    strb r4, [r2, #0x17]
    lsrs r3, r4, #16
    strb r3, [r2, #0x15]
    ldr r3, readRequestSettings
    str r3, [r2, #0x0C]

    ldr r5, probeTimeout
read_wait_request_done:
    ldrb r3, [r2, #0x0F]
    lsrs r3, r3, #8
    bcc read_begin_status
    subs r5, #1
    bne read_wait_request_done
    b read_one_failed

read_begin_status:
    ldr r0, probeTimeout    // Test 95: 0x20000 bounded E4 polls per sector
read_poll_status:
    subs r0, #1
    beq read_one_failed
    movs r3, #0xE4
    str r3, [r2, #0x10]
    movs r3, #0
    str r3, [r2, #0x14]
    ldr r3, statusTransferSettings
    str r3, [r2, #0x0C]

    ldr r5, probeTimeout
read_wait_status_data:
    ldrb r3, [r2, #0x0E]
    lsrs r3, r3, #8
    bcs read_status_ready
    subs r5, #1
    bne read_wait_status_data
    b read_one_failed

read_status_ready:
    ldr r3, cardDataReg
    ldr r3, [r3]
    cmp r3, #0
    bne read_fetch_sector

    ldr r5, probeTimeout
read_wait_status_done:
    ldrb r3, [r2, #0x0F]
    lsrs r3, r3, #8
    bcc read_poll_status
    subs r5, #1
    bne read_wait_status_done
    b read_one_failed

read_fetch_sector:
    ldr r5, probeTimeout
read_wait_before_e5:
    ldrb r3, [r2, #0x0F]
    lsrs r3, r3, #8
    bcc read_start_e5
    subs r5, #1
    bne read_wait_before_e5
    b read_one_failed

read_start_e5:
    movs r3, #0xE5
    str r3, [r2, #0x10]
    movs r3, #0
    str r3, [r2, #0x14]
    ldr r3, dataTransferSettings
    str r3, [r2, #0x0C]

    adr r1, loaderSectorBuffer
    movs r4, #128
read_store_words:
    ldr r5, probeTimeout
read_wait_word:
    ldrb r3, [r2, #0x0E]
    lsrs r3, r3, #8
    bcs read_word_ready
    subs r5, #1
    bne read_wait_word
    b read_one_failed

read_word_ready:
    ldr r3, cardDataReg
    ldr r3, [r3]
    str r3, [r1]
    adds r1, #4
    subs r4, #1
    bne read_store_words

    movs r0, #1
    pop {r4, pc}

read_one_failed:
    movs r0, #0
    pop {r4, pc}

.balign 4
.global patch_retailhotkeydetect_loaderExtents
patch_retailhotkeydetect_loaderExtents:
    .space 64               // seven count/start pairs plus zero terminator
loaderWordCount:
    .word 12120             // 48480 exact file bytes
expectedLoaderSum:
    .word 0x0E2D6140         // sum of all little-endian file words mod 2^32
cardRegBase:
    .word 0x04000198
cardDataReg:
    .word 0x04100010
readRequestSettings:
    .word 0xA0406000
statusTransferSettings:
    .word 0xA7446000
dataTransferSettings:
    .word 0xA1444000
probeTimeout:
    .word 0x00020000

.balign 4
loaderSectorBuffer:
    .space 512

.pool
.end
