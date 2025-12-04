.section ".itcm", "ax"
.arm

// r0: arm9EntryPoint
.global jumpToArm9EntryPoint
.type jumpToArm9EntryPoint, %function
jumpToArm9EntryPoint:
    str r0, entry_point
    // Clear all registers
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    mov sp, #0
    mov lr, #0
    // Jump to the arm9 entry point
    ldr pc, entry_point

entry_point:
    .word 0
