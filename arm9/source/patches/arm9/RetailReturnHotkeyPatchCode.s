.cpu arm946e-s
.section "patch_retailreturnhotkey", "ax"
.syntax unified

.arm
.global patch_retailreturnhotkey_entry
.type patch_retailreturnhotkey_entry, %function
patch_retailreturnhotkey_entry:
    stmdb sp!, {r0-r3, r12, lr}

    // ARM7 posts "PICO" into shared RAM after L+R+Down+Select is held.
    // This VBlank-table responder only acknowledges it; ARM7 remains the
    // owner of the hotkey and decides what happens next.
    ldr r0, retailReturnMarkerAddress
    ldr r1, [r0]
    ldr r2, markerValue
    cmp r1, r2
    bne chain_original

    ldr r1, ackValue
    str r1, [r0]

chain_original:
    ldmia sp!, {r0-r3, r12, lr}

.global patch_retailreturnhotkey_original_instruction0
patch_retailreturnhotkey_original_instruction0:
    .word 0
.global patch_retailreturnhotkey_original_instruction1
patch_retailreturnhotkey_original_instruction1:
    .word 0

    // Tail-chain to the game's original VBlank callback. LR is still the
    // dispatcher return address supplied by the Nintendo SDK IRQ code.
    ldr pc, patch_retailreturnhotkey_return_address

.balign 4
retailReturnMarkerAddress:
    .word 0x02FFFDF0
markerValue:
    .word 0x5049434F
ackValue:
    .word 0x41434B21

.global patch_retailreturnhotkey_return_address
patch_retailreturnhotkey_return_address:
    .word 0
.global patch_retailreturnhotkey_reset_address
patch_retailreturnhotkey_reset_address:
    .word 0

.pool
.end
