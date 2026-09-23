.cpu arm7tdmi
.section "patch_readuserinfo_language", "ax"
.syntax unified
.arm

// Replaces "mov r2, #0x74; bl MI_CpuCopy32; b next" in the arm7 ReadUserInfo.
// r0 = source user settings
// r1 = nvramUserInfo
// r2 = address of the replaced "b next"
.global readuserinfo_language_copyUserInfo
.type readuserinfo_language_copyUserInfo, %function
readuserinfo_language_copyUserInfo:
    mov r3, #0x74 // sizeof(NVRAMConfig)
1:
    subs r3, r3, #4
    ldr r12, [r0, r3]
    str r12, [r1, r3]
    bne 1b

    // language is in bits 0-2 of the language and flags field
    ldrb r3, [r1, #0x64]
    bic r3, r3, #7
.global readuserinfo_language_orrLanguageInstruction
readuserinfo_language_orrLanguageInstruction:
    orr r3, r3, #0 // immediate is set to the language when patching
    strb r3, [r1, #0x64]

.global readuserinfo_language_returnInstruction
readuserinfo_language_returnInstruction:
    add pc, r2, #0 // immediate is set to the offset of "next" when patching

.end
