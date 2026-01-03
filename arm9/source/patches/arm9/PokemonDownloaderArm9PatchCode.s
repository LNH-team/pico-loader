.cpu arm946e-s
.section "patch_pokemondownloader9", "ax"
.syntax unified

.thumb
.global patch_pokemondownloader9_entry
.type patch_pokemondownloader9_entry, %function
patch_pokemondownloader9_entry:
    ldr r0,= 0x04000204
    ldrh r1, [r0]
    movs r2, #1
    lsls r2, r2, #11
    bics r1, r2
    strh r1, [r0]

    adr r0, regVramCntA
    // regVramCntA, vramAbcdLcdcSetting, readSdSectors_address, loader_info_address
    ldmia r0!, {r2, r3, r4, r5}

    // r9 = readSdSectors_address
    mov r9, r4

    // map vram ABCD to LCDC
    str r3, [r2]

    // load pico loader arm9
    ldmia r5!, {r4,r6,r7} // clusterShift, database, clusterMap[0]
    ldr r7,= 0x06800000
    mov r11, pc
    b loadData

    // load pico loader arm7
    ldr r5, patch_pokemondownloader9_loader_info_address
    adds r5, #52
    ldr r7,= 0x06840000
    mov r11, pc
    b loadData

    ldr r3, patch_pokemondownloader9_loader_info_address
    ldrh r0, [r3, #2] // loader_info_t::picoLoaderBootDrive
    movs r7, #1
    lsls r7, r7, #15 // PLOAD_BOOT_DRIVE_MULTIBOOT_FLAG
    orrs r0, r7
    ldr r5,= 0x06840000
    strh r0, [r5, #8] // pload_header7_t::bootDrive

    // map vram CD to arm7
    ldr r0, regVramCntA
    ldr r7,= 0x8A82
    strh r7, [r0, #2]

    adds r0, #(0x04000180 - 0x04000240) // REG_IPC_SYNC

1:
    ldrb r7, [r0] // ipc sync
    cmp r7, #1
    bne 1b // while ipc sync from arm7 is not 1

    movs r1, #1
    strb r1, [r0, #1]

    ldr r0,= 0x06800000
    bx r0

loadData_loop:
    subs r3, #2
    lsls r3, r4
    adds r0, r3, r6 // start sector
    movs r1, r7 // dst
    lsls r2, r4 // sector count
    lsls r3, r2, #9
    adds r7, r3

    blx r9 // read sectors

loadData:
    ldmia r5!, {r2, r3} // ncl, startSector
    cmp r2, #0
    bne loadData_loop
    mov pc, r11

.balign 4

regVramCntA:
    .word 0x04000240

vramAbcdLcdcSetting:
    .word 0x80808080

.global patch_pokemondownloader9_readSdSectors_address
patch_pokemondownloader9_readSdSectors_address:
    .word 0

.global patch_pokemondownloader9_loader_info_address
patch_pokemondownloader9_loader_info_address:
    .word 0

.pool
.end
