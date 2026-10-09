.cpu arm946e-s
.syntax unified
.section "patch_waitsplit_probe", "ax"
.arm
.global waitSplitProbeEntry
.type waitSplitProbeEntry, %function
waitSplitProbeEntry:
    stmdb sp!, {r0-r3,r12,lr}
    mrs r2, cpsr
    stmdb sp!, {r2,r3}
    and r3, r2, #0x1F
    cmp r3, #0x1F
    bne splitProbeReturn
    ldr r0, splitProbeIpc
    ldrh r1, [r0]
    and r1, r1, #15
    cmp r1, #14
    bne splitProbeReturn
    ldr r0, splitProbeRomCtrl
    ldr r1, [r0]
    tst r1, #0x80000000
    bne splitProbeReturn
    ldr pc, waitSplitRequestAddress
splitProbeReturn:
    ldr pc, waitSplitReturnAddress
splitProbeIpc: .word 0x04000180
splitProbeRomCtrl: .word 0x040001A4
.global waitSplitRequestAddress
waitSplitRequestAddress: .word 0
.global waitSplitReturnAddress
waitSplitReturnAddress: .word 0

.section "patch_waitsplit_resume", "ax"
.arm
.global waitSplitRequestEntry
.type waitSplitRequestEntry, %function
waitSplitRequestEntry:
    ldr r0, splitResumeIme
    mov r1, #0
    str r1, [r0]
    ldr r3, waitSplitFix
    blx r3
    adr r3, splitWaitTakeover + 1
    bx r3
.global waitSplitReturnEntry
.type waitSplitReturnEntry, %function
waitSplitReturnEntry:
    ldmia sp!, {r2,r3}
    msr cpsr_f, r2
    ldmia sp!, {r0-r3,r12,lr}
    bx lr
splitResumeIme: .word 0x04000208
.global waitSplitFix
waitSplitFix: .word 0
.thumb
.type splitWaitTakeover, %function
splitWaitTakeover:
    push {r1,r2,r3,r4,r6,lr}
    ldr r3, waitSplitTakeover
    bx r3
.balign 4
.global waitSplitTakeover
waitSplitTakeover: .word 0
.end
