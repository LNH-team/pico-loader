.cpu arm946e-s
.section "patch_lastwindowcrc", "ax"
.syntax unified

.thumb
.global patch_lastwindowcrc_entry
.type patch_lastwindowcrc_entry, %function
patch_lastwindowcrc_entry:
    // r0=crc_start, r1=datap, r2=size
    ldr r3, =0x0207D4F4 // CRC of DS Protect region
    cmp r1, r3
    beq crc_dsprotect

    ldr r3, =0x0202DB34 // CRC of other code region (unknown)
    cmp r1, r3
    beq crc_unk

    swi #0xE // Normal CRC if address does not match (never hit?)
    bx lr

crc_dsprotect:
    ldr r0, =0x67D7
    bx lr

crc_unk:
    ldr r0, =0x0D1E
    bx lr

.pool
.end
