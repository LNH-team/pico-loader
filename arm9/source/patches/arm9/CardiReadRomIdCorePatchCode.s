.cpu arm946e-s
.section "patch_cardireadromidcore", "ax"
.syntax unified

.thumb
.global patch_cardireadromidcore_entry
.type patch_cardireadromidcore_entry, %function
patch_cardireadromidcore_entry:
    ldr r0, patch_cardireadromidcore_cardid_address
    ldr r0, [r0]
    bx lr

.balign 4

.global patch_cardireadromidcore_cardid_address
patch_cardireadromidcore_cardid_address:
    .word 0x027FFC00

.pool

.end