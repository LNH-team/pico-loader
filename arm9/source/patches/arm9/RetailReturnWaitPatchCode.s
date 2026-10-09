.cpu arm946e-s
.section "patch_retailreturnwait", "ax"
.syntax unified
.arm
.global patch_retailreturnwait_entry
.type patch_retailreturnwait_entry, %function
patch_retailreturnwait_entry:
    stmdb sp!, {r0-r3,r12,lr}
    mrs r2, cpsr
    stmdb sp!, {r2,r3}
    // Only take over from a game thread in system mode, never an IRQ/SVC context.
    and r3, r2, #0x1F
    cmp r3, #0x1F
    bne waitReturn
    ldr r0, waitIpc
    ldrh r1, [r0]
    and r1, r1, #15
    cmp r1, #14
    bne waitReturn
    ldr r0, waitRomCtrl
    ldr r1, [r0]
    tst r1, #0x80000000
    bne waitReturn
    // Takeover only after the hotkey request; no new IRQ callback.
    ldr r0, waitIme
    mov r1, #0
    str r1, [r0]
    ldr r3, patch_retailreturnwait_fix
    blx r3
    adr r3, waitTakeover + 1
    bx r3
waitReturn:
    ldmia sp!, {r2,r3}
    msr cpsr_f, r2
    ldmia sp!, {r0-r3,r12,lr}
    bx lr
waitIpc:
    .word 0x04000180
waitRomCtrl:
    .word 0x040001A4
waitIme:
    .word 0x04000208
.global patch_retailreturnwait_fix
patch_retailreturnwait_fix:
    .word 0
.thumb
.type waitTakeover, %function
waitTakeover:
    // Match the existing CARDi epilogue's stack shape (request never returns).
    push {r1,r2,r3,r4,r6,lr}
    ldr r3, patch_retailreturnwait_takeover
    bx r3
.balign 4
.global patch_retailreturnwait_takeover
patch_retailreturnwait_takeover:
    .word 0
.end
