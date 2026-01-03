.cpu arm7tdmi
.syntax unified
.section "r4idsn_readsd", "ax"
.thumb

// r0 = src sector
// r1 = dst
// r2 = sector count
.global r4idsn_readSd
.type r4idsn_readSd, %function
r4idsn_readSd:
    push {r4,lr}

    ldr r4, =0x040001A0
    movs r3, #0x80
    strb r3, [r4,#1]

.global r4idsn_readSd_sdsc_shift
r4idsn_readSd_sdsc_shift:
    lsls r4, r0, #9

    push {r0-r1}

    ldr r0, =0x120004AB
    movs r1, r4
    ldr r3, r4idsn_readSd_sendSdioCommand_address
    bl blx_r3

    // Wait for SD state
    movs r0, #4
    movs r1, #7
    ldr r3, r4idsn_readSd_sdWaitForState_address
    bl blx_r3

    pop {r0-r1}

    // Read sectors. Parameters identical to readSd
    ldr r3, r4idsn_readSd_sdReadSector_address
    bl blx_r3

    // Send CMD12 == STOP_TRANSMISSION
    push {r0-r1}
    ldr r0, =0x0C0001AB
    movs r1, #0
    ldr r3, r4idsn_readSd_sendSdioCommand_address
    bl blx_r3

    pop {r0-r1,r4,pc}

blx_r3:
    bx r3

.balign 4

.global r4idsn_readSd_sendSdioCommand_address
r4idsn_readSd_sendSdioCommand_address:
    .word 0

.global r4idsn_readSd_sdReadSector_address
r4idsn_readSd_sdReadSector_address:
    .word 0

.global r4idsn_readSd_sdWaitForState_address
r4idsn_readSd_sdWaitForState_address:
    .word 0

.pool

.end
